//
// Created by qinshenghua on 2020/12/15.
//

#include "DemuxerHelper.h"
#include "../base/Log.h"

#define TAG "Demuxer"

static void print_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag) {
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    LOGD(TAG, "%s, flags:%d, pts:%lld pts_time:%s dts_time:%s size:%d\n",
         tag, pkt->flags,
         (uint64_t) (av_q2d(*time_base) * pkt->pts * 1000000),
         av_ts2timestr(pkt->pts, time_base), av_ts2timestr(pkt->dts, time_base),
         pkt->size);
}

void
DemuxerHelper::start(char *filename, GPlayer *player, FormatInfo *formatInfo) {
    LOGI(TAG, "CoreFlow : ffmpeg_demuxing '%s'", filename);
    AVFormatContext *ifmt_ctx = nullptr;
    AVPacket pkt;
    int ret;
    int i;
    int stream_index = 0;
    int audio_stream_index = -1;
    int video_stream_index = -1;
    int subtitle_stream_index = -1;
    int *stream_mapping = nullptr;
    uint32_t stream_mapping_size = 0;

    int64_t seekUs = -1;
    AVCodecParameters *video_codecpar = nullptr;
    AVCodecParameters *audio_codecpar = nullptr;
    int needVideoStreamFilter = 0;
    int needAudioStreamFilter = 0;
    unsigned char mADTSHeader[ADTS_HEADER_SIZE] = {0};
    AVBSFContext *video_abs_ctx = nullptr;
    const AVBitStreamFilter *video_abs_filter = nullptr;

    av_register_all();
    if ((ret = avformat_open_input(&ifmt_ctx, filename, 0, 0)) < 0) {
        LOGE(TAG, "Could not open input file '%s'", filename);
        goto end;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        LOGE(TAG, "Failed to retrieve input stream information");
        goto end;
    }

    av_dump_format(ifmt_ctx, 0, filename, 0);

    stream_mapping_size = ifmt_ctx->nb_streams;
    stream_mapping = static_cast<int *>(av_mallocz_array((size_t) stream_mapping_size,
                                                         sizeof(*stream_mapping)));
    if (!stream_mapping) {
        ret = AVERROR(ENOMEM);
        LOGE(TAG, "invalid stream_mapping");
        goto end;
    }

    for (i = 0; i < stream_mapping_size; i++) {
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;

        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_mapping[i] = -1;
            LOGE(TAG, "Error: invalid media type %d", in_codecpar->codec_type);
            continue;
        }
        if (in_codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (video_stream_index == -1) {
                video_stream_index = i;
                avcodec_parameters_copy(formatInfo->vidcodecpar, in_codecpar);
                formatInfo->vidframerate = (int) (
                        in_stream->r_frame_rate.num / in_stream->r_frame_rate.den + 0.5f);
                AVDictionaryEntry *tag = nullptr;
                tag = av_dict_get(in_stream->metadata, "rotate", tag, AV_DICT_IGNORE_SUFFIX);
                formatInfo->vidrotate = tag != nullptr ? atoi(tag->value) : 0;
            } else {
                LOGE(TAG, "Error: video_stream_index has been set to %d", video_stream_index);
            }
        } else if (in_codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audio_stream_index == -1) {
                audio_stream_index = i;
                avcodec_parameters_copy(formatInfo->audcodecpar, in_codecpar);
            } else {
                LOGE(TAG, "Error: audio_stream_index has been set to %d", audio_stream_index);
            }
        } else if (in_codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
            if (subtitle_stream_index == -1) {
                subtitle_stream_index = i;
                avcodec_parameters_copy(formatInfo->subcodecpar, in_codecpar);
            } else {
                LOGE(TAG, "Error: subtitle_stream_index has been set to %d", subtitle_stream_index);
            }
        }

        stream_mapping[i] = stream_index++;
    }

    LOGI(TAG, "audio_stream_index = %d, video_stream_index = %d, stream_mapping_size = %d\n",
         audio_stream_index, video_stream_index, stream_mapping_size);
    LOGI(TAG, "extensions %s", ifmt_ctx->iformat->extensions);

    LOGI(TAG, "av_init\n");
    formatInfo->duration = (int) (ifmt_ctx->duration);
    player->av_init(formatInfo);

    video_codecpar = ifmt_ctx->streams[video_stream_index]->codecpar;
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

    audio_codecpar = ifmt_ctx->streams[audio_stream_index]->codecpar;
    needAudioStreamFilter = ifmt_ctx->iformat->extensions != NULL &&
                            audio_codecpar->codec_id == AV_CODEC_ID_AAC;
    ADTSContext mADTSContext;
    if (needAudioStreamFilter) {
        create_adts_context(&mADTSContext, audio_codecpar->extradata,
                            audio_codecpar->extradata_size);
    }
    LOGI(TAG, "CoreFlow : needVideoStreamFilter %d, needAudioStreamFilter %d\n",
         needVideoStreamFilter, needAudioStreamFilter);
    av_init_packet(&pkt);
    pkt.data = nullptr;
    pkt.size = 0;
    while (1) {
        LoopFlag loop_flag = player->loopWait(&seekUs);
        if (loop_flag == CONTINUE) {
            continue;
        } else if (loop_flag == BREAK) {
            break;
        }
        if (seekUs != -1) {
            LOGI(TAG, "start time %lld, seekUs %lld", ifmt_ctx->start_time, seekUs);
            av_seek_frame(ifmt_ctx, -1, seekUs * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);
            seekUs = -1;
        }
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
            break;

        if (pkt.stream_index >= stream_mapping_size ||
            stream_mapping[pkt.stream_index] < 0) {
            av_packet_unref(&pkt);
            continue;
        }

        if (pkt.flags & AV_PKT_FLAG_DISCARD) {
            av_packet_unref(&pkt);
            continue;
        }

        pkt.stream_index = stream_mapping[pkt.stream_index];
        if (pkt.stream_index == audio_stream_index) {
            print_packet(ifmt_ctx, &pkt, "audio");
            if (needAudioStreamFilter) {
                insert_adts_head(&mADTSContext, mADTSHeader, static_cast<uint32_t>(pkt.size));
                AVPacket *audPkt = av_packet_alloc();
                av_packet_copy_props(audPkt, &pkt);
                audPkt->size = pkt.size + ADTS_HEADER_SIZE;
                audPkt->data = static_cast<uint8_t *>(av_malloc(static_cast<size_t>(audPkt->size)));
                memcpy(audPkt->data, mADTSHeader, ADTS_HEADER_SIZE);
                memcpy(audPkt->data + ADTS_HEADER_SIZE, pkt.data, (size_t) pkt.size);
                player->av_feed_audio(audPkt, ifmt_ctx->streams[pkt.stream_index]->time_base);
                av_packet_free(&audPkt);
            } else {
                player->av_feed_audio(&pkt, ifmt_ctx->streams[pkt.stream_index]->time_base);
            }
        } else if (pkt.stream_index == video_stream_index) {
            print_packet(ifmt_ctx, &pkt, "video");
            if (needVideoStreamFilter) {
                av_bsf_send_packet(video_abs_ctx, &pkt);
                while (av_bsf_receive_packet(video_abs_ctx, &pkt) == 0) {
                    if (pkt.size <= 0) {
                        continue;
                    }
                    player->av_feed_video(&pkt, ifmt_ctx->streams[pkt.stream_index]->time_base);
                }
            } else {
                player->av_feed_video(&pkt, ifmt_ctx->streams[pkt.stream_index]->time_base);
            }
        }

        if (ret < 0) {
            LOGE(TAG, "Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);
    }

    LOGI(TAG, "av_destroy\n");
    player->av_destroy();

    end:

    avformat_close_input(&ifmt_ctx);

    av_freep(&stream_mapping);

    if (needVideoStreamFilter) {
        av_bsf_free(&video_abs_ctx);
        video_abs_ctx = NULL;
    }

    if (ret < 0 && ret != AVERROR_EOF) {
        LOGE(TAG, "Error occurred: %s\n", av_err2str(ret));
        player->av_error(ret, av_err2str(ret));
    }
    LOGI(TAG, "ending");
}
