//
// Created by qinshenghua on 2020/12/15.
//

#include "DemuxerHelper.h"
#include "../base/Log.h"

#define TAG "DemuxerHelper"

static void print_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag) {
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    LOGD(TAG, "%s, flags:%d, pts:%lld pts_time:%s dts_time:%s size:%d\n",
         tag, pkt->flags,
         (uint64_t) (av_q2d(*time_base) * pkt->pts * 1000000),
         av_ts2timestr(pkt->pts, time_base), av_ts2timestr(pkt->dts, time_base),
         pkt->size);
}

DemuxerHelper::DemuxerHelper(char *url, PacketSource *input, MessageQueue *messageQueue) {
    filename = url;
    inputSource = input;
    formatInfo = inputSource->getFormatInfo();
    this->messageQueue = messageQueue;
}

void DemuxerHelper::init() {
    av_register_all();
    int ret;
    if ((ret = avformat_open_input(&ifmt_ctx, filename, nullptr, nullptr)) < 0) {
        LOGE(TAG, "Could not open input file '%s'", filename);
        notifyError(ret);
        return;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, nullptr)) < 0) {
        LOGE(TAG, "Failed to retrieve input stream information");
        notifyError(ret);
        return;
    }

    av_dump_format(ifmt_ctx, 0, filename, 0);

    uint32_t stream_mapping_size = ifmt_ctx->nb_streams;

    for (int i = 0; i < stream_mapping_size; i++) {
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;

        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            LOGE(TAG, "Error: invalid media type %d", in_codecpar->codec_type);
            continue;
        }
        if (in_codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (video_stream_index == -1) {
                video_stream_index = in_stream->index;
                avcodec_parameters_copy(formatInfo->vidcodecpar, in_codecpar);
                formatInfo->vidframerate = static_cast<int>(lround(
                        in_stream->r_frame_rate.num / in_stream->r_frame_rate.den));
                AVDictionaryEntry *tag = nullptr;
                tag = av_dict_get(in_stream->metadata, "rotate", tag, AV_DICT_IGNORE_SUFFIX);
                formatInfo->vidrotate = tag != nullptr ? atoi(tag->value) : 0;
            } else {
                LOGE(TAG, "Error: video_stream_index has been set to %d", video_stream_index);
            }
        } else if (in_codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audio_stream_index == -1) {
                audio_stream_index = in_stream->index;
                avcodec_parameters_copy(formatInfo->audcodecpar, in_codecpar);
            } else {
                LOGE(TAG, "Error: audio_stream_index has been set to %d", audio_stream_index);
            }
        } else if (in_codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
            if (subtitle_stream_index == -1) {
                subtitle_stream_index = in_stream->index;
                avcodec_parameters_copy(formatInfo->subcodecpar, in_codecpar);
            } else {
                LOGE(TAG, "Error: subtitle_stream_index has been set to %d", subtitle_stream_index);
            }
        }
    }

    LOGI(TAG, "audio_stream_index = %d, video_stream_index = %d, stream_mapping_size = %d\n",
         audio_stream_index, video_stream_index, stream_mapping_size);
    LOGI(TAG, "extensions %s", ifmt_ctx->iformat->extensions);

    LOGI(TAG, "av_init\n");
    formatInfo->duration = (int) (ifmt_ctx->duration);
    inputSource->queueInfo(formatInfo);

    AVCodecParameters *video_codecpar = ifmt_ctx->streams[video_stream_index]->codecpar;
    needVideoStreamFilter = ifmt_ctx->iformat->extensions != nullptr &&
                            (video_codecpar->codec_id == AV_CODEC_ID_H264 ||
                             video_codecpar->codec_id == AV_CODEC_ID_HEVC);
    if (needVideoStreamFilter) {
        if (video_codecpar->codec_id == AV_CODEC_ID_HEVC) {
            video_abs_filter = av_bsf_get_by_name("hevc_mp4toannexb");
        } else if (video_codecpar->codec_id == AV_CODEC_ID_H264) {
            video_abs_filter = av_bsf_get_by_name("h264_mp4toannexb");
        }
        av_bsf_alloc(video_abs_filter, &video_abs_ctx);
        avcodec_parameters_copy(video_abs_ctx->par_in, video_codecpar);
        av_bsf_init(video_abs_ctx);
    }

    AVCodecParameters *audio_codecpar = ifmt_ctx->streams[audio_stream_index]->codecpar;
    needAudioStreamFilter = ifmt_ctx->iformat->extensions != nullptr &&
                            audio_codecpar->codec_id == AV_CODEC_ID_AAC;
    if (needAudioStreamFilter) {
        create_adts_context(&mADTSContext, audio_codecpar->extradata,
                            audio_codecpar->extradata_size);
    }
    LOGI(TAG, "CoreFlow : needVideoStreamFilter %d, needAudioStreamFilter %d\n",
         needVideoStreamFilter, needAudioStreamFilter);
}

int DemuxerHelper::update(int type, long extra) {
    int64_t seekUs = extra;
    if (seekUs != -1) {
        LOGI(TAG, "start time %lld, seekUs %lld", ifmt_ctx->start_time, seekUs);
        av_seek_frame(ifmt_ctx, -1, seekUs * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);
    }

    AVPacket pkt;
    int ret = av_read_frame(ifmt_ctx, &pkt);
    if (ret < 0) {
        notifyError(ret);
        return 0;
    }

    if (pkt.flags & AV_PKT_FLAG_DISCARD) {
        av_packet_unref(&pkt);
        return 0;
    }

    if (pkt.stream_index == audio_stream_index) {
//        print_packet(ifmt_ctx, &pkt, "audio");
        if (needAudioStreamFilter) {
            insert_adts_head(&mADTSContext, mADTSHeader, static_cast<uint32_t>(pkt.size));
            AVPacket *audPkt = av_packet_alloc();
            av_packet_copy_props(audPkt, &pkt);
            audPkt->size = pkt.size + ADTS_HEADER_SIZE;
            audPkt->data = static_cast<uint8_t *>(av_malloc(static_cast<size_t>(audPkt->size)));
            memcpy(audPkt->data, mADTSHeader, ADTS_HEADER_SIZE);
            memcpy(audPkt->data + ADTS_HEADER_SIZE, pkt.data, (size_t) pkt.size);
            inputSource->queueAudPkt(audPkt, ifmt_ctx->streams[pkt.stream_index]->time_base);
            av_packet_free(&audPkt);
        } else {
            inputSource->queueAudPkt(&pkt, ifmt_ctx->streams[pkt.stream_index]->time_base);
        }
    } else if (pkt.stream_index == video_stream_index) {
//        print_packet(ifmt_ctx, &pkt, "video");
        if (needVideoStreamFilter) {
            av_bsf_send_packet(video_abs_ctx, &pkt);
            while (av_bsf_receive_packet(video_abs_ctx, &pkt) == 0) {
                if (pkt.size <= 0) {
                    continue;
                }
                inputSource->queueVidPkt(&pkt, ifmt_ctx->streams[pkt.stream_index]->time_base);
            }
        } else {
            inputSource->queueVidPkt(&pkt, ifmt_ctx->streams[pkt.stream_index]->time_base);
        }
    }

    av_packet_unref(&pkt);
    return 0;
}

void DemuxerHelper::release() {
    LOGI(TAG, "av_destroy\n");
    avformat_close_input(&ifmt_ctx);

    if (needVideoStreamFilter) {
        av_bsf_free(&video_abs_ctx);
        video_abs_ctx = nullptr;
    }

    messageQueue->pushMessage(MSG_FROM_DEMUXING, MSG_DEMUXING_DESTROY, 0);
}

void DemuxerHelper::notifyError(int ret) {
    messageQueue->pushMessage(MSG_FROM_DEMUXING, MSG_DEMUXING_ERROR, ret);
}
