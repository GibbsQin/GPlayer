#include <android/log.h>
#include <codec/ffmpeg/libavutil/timestamp.h>
#include <unistd.h>
#include "demuxing.h"

#define LOG_LEVEL ANDROID_LOG_INFO

static void ffmpegLog(int level, const char *fmt, ...) {
    if (level < LOG_LEVEL) {
        return;
    }
    char log_info[2040];
    char *buf = log_info;
    int ret, len = sizeof(log_info);

    va_list arglist;
    va_start(arglist, fmt);

    ret = vsnprintf(buf, len - 1, fmt, arglist);
    if (ret < 0) {
        buf[len - 1] = 0;
        buf[len - 2] = '\n';
    }

    va_end(arglist);

    __android_log_print(level, "ffmpeg_remuxing", log_info, "");
}

uint64_t ffmpeg_pts2timeus(AVRational time_base, int64_t pts) {
    return (uint64_t) (av_q2d(time_base) * pts * 1000000);
}

static void print_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag) {
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    ffmpegLog(ANDROID_LOG_DEBUG, "%s, flags:%d, pts:%lld pts_time:%s dts_time:%s size:%d\n",
              tag, pkt->flags,
              ffmpeg_pts2timeus(*time_base, pkt->pts),
              av_ts2timestr(pkt->pts, time_base), av_ts2timestr(pkt->dts, time_base),
              pkt->size);
}

void ffmpeg_demuxing(char *filename, int channelId, FfmpegCallback callback, MediaInfo *mediaInfo) {
    AVFormatContext *ifmt_ctx = NULL;
    AVPacket pkt;
    int ret;
    int i;
    int stream_index = 0;
    int audio_stream_index = -1;
    int video_stream_index = -1;
    int *stream_mapping = NULL;
    uint32_t stream_mapping_size = 0;

    av_register_all();
    if ((ret = avformat_open_input(&ifmt_ctx, filename, 0, 0)) < 0) {
        ffmpegLog(ANDROID_LOG_ERROR, "Could not open input file '%s'", filename);
        goto end;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        ffmpegLog(ANDROID_LOG_ERROR, "Failed to retrieve input stream information");
        goto end;
    }

    av_dump_format(ifmt_ctx, 0, filename, 0);

    stream_mapping_size = ifmt_ctx->nb_streams;
    stream_mapping = av_mallocz_array((size_t) stream_mapping_size, sizeof(*stream_mapping));
    if (!stream_mapping) {
        ret = AVERROR(ENOMEM);
        ffmpegLog(ANDROID_LOG_ERROR, "invalid stream_mapping");
        goto end;
    }

    for (i = 0; i < stream_mapping_size; i++) {
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;

        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_mapping[i] = -1;
            ffmpegLog(ANDROID_LOG_ERROR, "Error: invalid media type %d", in_codecpar->codec_type);
            continue;
        }
        if (in_codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (video_stream_index == -1) {
                video_stream_index = i;
            } else {
                ffmpegLog(ANDROID_LOG_ERROR, "Error: video_stream_index has been set to %d", video_stream_index);
            }
        } else if (in_codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audio_stream_index == -1) {
                audio_stream_index = i;
            } else {
                ffmpegLog(ANDROID_LOG_ERROR, "Error: audio_stream_index has been set to %d", audio_stream_index);
            }
        } else if (in_codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
            ffmpegLog(ANDROID_LOG_ERROR, "Error: subtitle media type");
        }

        stream_mapping[i] = stream_index++;
    }

    ffmpegLog(ANDROID_LOG_INFO, "audio_stream_index = %d, video_stream_index = %d, stream_mapping_size = %d\n",
         audio_stream_index, video_stream_index, stream_mapping_size);
    ffmpegLog(ANDROID_LOG_INFO, "extensions %s", ifmt_ctx->iformat->extensions);

    ffmpegLog(ANDROID_LOG_INFO, "av_init\n");
    callback.av_format_init(channelId, ifmt_ctx, ifmt_ctx->streams[audio_stream_index],
                            ifmt_ctx->streams[video_stream_index], mediaInfo);
    //send SPS PPS
    callback.av_format_extradata_video(channelId, ifmt_ctx,
                                       ifmt_ctx->streams[video_stream_index]->codecpar->extradata,
                                       (uint32_t) ifmt_ctx->streams[video_stream_index]->codecpar->extradata_size);

    callback.av_format_extradata_audio(channelId, ifmt_ctx,
                                       ifmt_ctx->streams[audio_stream_index]->codecpar->extradata,
                                       (uint32_t) ifmt_ctx->streams[audio_stream_index]->codecpar->extradata_size);

    int needStreamFilter = ifmt_ctx->iformat->extensions != NULL;
    AVBSFContext *video_abs_ctx = NULL;
    const AVBitStreamFilter *video_abs_filter = NULL;
    if (needStreamFilter) {
        AVCodecParameters *in_codecpar = ifmt_ctx->streams[video_stream_index]->codecpar;
        if (in_codecpar->codec_id == AV_CODEC_ID_HEVC) {
            video_abs_filter = av_bsf_get_by_name("hevc_mp4toannexb");
        } else if (in_codecpar->codec_id == AV_CODEC_ID_H264) {
            video_abs_filter = av_bsf_get_by_name("h264_mp4toannexb");
        }
        av_bsf_alloc(video_abs_filter, &video_abs_ctx);
        avcodec_parameters_copy(video_abs_ctx->par_in, in_codecpar);
        av_bsf_init(video_abs_ctx);
    }

    while (1) {
        LoopFlag loop_flag = callback.av_format_loop_wait(channelId);
        if (loop_flag == CONTINUE) {
            continue;
        } else if (loop_flag == BREAK) {
            break;
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
            callback.av_format_feed_audio(channelId, ifmt_ctx, &pkt);
        } else if (pkt.stream_index == video_stream_index) {
            print_packet(ifmt_ctx, &pkt, "video");
            if (needStreamFilter) {
                av_bsf_send_packet(video_abs_ctx, &pkt);
                while (av_bsf_receive_packet(video_abs_ctx, &pkt) == 0) {
                    if (pkt.size <= 0) {
                        continue;
                    }
                    callback.av_format_feed_video(channelId, ifmt_ctx, &pkt);
                }
            } else {
                callback.av_format_feed_video(channelId, ifmt_ctx, &pkt);
            }
        }

        if (ret < 0) {
            ffmpegLog(ANDROID_LOG_ERROR, "Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);
    }

    ffmpegLog(ANDROID_LOG_INFO, "av_destroy\n");
    callback.av_format_destroy(channelId, ifmt_ctx);

    end:

    avformat_close_input(&ifmt_ctx);

    av_freep(&stream_mapping);

    if (needStreamFilter) {
        av_bsf_free(&video_abs_ctx);
        video_abs_ctx = NULL;
    }

    if (ret < 0 && ret != AVERROR_EOF) {
        ffmpegLog(ANDROID_LOG_ERROR, "Error occurred: %s\n", av_err2str(ret));
        callback.av_format_error(channelId, ret, av_err2str(ret));
    }
    ffmpegLog(ANDROID_LOG_INFO, "ending");
}

void ffmpeg_extra_audio_info(AVFormatContext *ifmt_ctx, AVStream *stream, MediaInfo *mediaInfo) {
    mediaInfo->duration = (int) (ifmt_ctx->duration / 1000);
    AVCodecParameters *in_codecpar = stream->codecpar;
    mediaInfo->audioType = in_codecpar->codec_id;
    mediaInfo->audioProfile = in_codecpar->profile;
    mediaInfo->audioMode = (int) in_codecpar->channel_layout;
    mediaInfo->audioChannels = in_codecpar->channels;
    mediaInfo->audioBitWidth = in_codecpar->format;
    mediaInfo->audioSampleRate = in_codecpar->sample_rate;
    mediaInfo->sampleNumPerFrame = in_codecpar->frame_size;
}

void ffmpeg_extra_video_info(AVFormatContext *ifmt_ctx, AVStream *stream, MediaInfo *mediaInfo) {
    AVCodecParameters *in_codecpar = stream->codecpar;
    mediaInfo->videoType = in_codecpar->codec_id;
    mediaInfo->videoWidth = in_codecpar->width;
    mediaInfo->videoHeight = in_codecpar->height;
    mediaInfo->videoFrameRate = (stream->r_frame_rate.num / stream->r_frame_rate.den + 0.5f);
    AVDictionaryEntry *tag = NULL;
    tag = av_dict_get(stream->metadata, "rotate", tag, AV_DICT_IGNORE_SUFFIX);
    if (tag != NULL) {
        mediaInfo->videoRotate = atoi(tag->value);
    } else {
        mediaInfo->videoRotate = 0;
    }
}