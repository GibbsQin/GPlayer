#include <pthread.h>
#include <stdbool.h>
#include <malloc.h>
#include <android/log.h>
#include <media/Media.h>
#include <codec/ffmpeg/libavutil/timestamp.h>
#include "stream_protocol.h"
#include "../codec/ffmpeg/libavutil/rational.h"
#include "../codec/ffmpeg/libavformat/avformat.h"
#include "../codec/ffmpeg/libavcodec/avcodec.h"
#include "../codec/ffmpeg/libavutil/avutil.h"

#define DEBUG_LOG 1

static uint64_t pts2timeus(AVRational time_base, int64_t pts) {
    return (uint64_t) (av_q2d(time_base) * pts * 1000000);
}

static void logD(const char *fmt, ...) {
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

    __android_log_print(2, "demuxing", log_info, "");
}

static void print_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag) {
#ifdef DEBUG_LOG
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    logD("%s, flags:%d, pts:%lld pts_time:%s dts_time:%s size:%d\n",
         tag, pkt->flags,
         pts2timeus(*time_base, pkt->pts),
         av_ts2timestr(pkt->pts, time_base), av_ts2timestr(pkt->dts, time_base),
         pkt->size);
#endif
}

void start_demuxing(const char *in_filename, const struct MediaCallback media_callback,
                    const int channelId, is_demuxing loop) {
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
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        logD("Could not open input file '%s'", in_filename);
        goto end;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        logD("Failed to retrieve input stream information");
        goto end;
    }

    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    stream_mapping_size = ifmt_ctx->nb_streams;
    stream_mapping = av_mallocz_array((size_t) stream_mapping_size, sizeof(*stream_mapping));
    if (!stream_mapping) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    MediaInfo *header = malloc(sizeof(MediaInfo));
    header->duration = (int) (ifmt_ctx->duration / 1000);
    header->videoType = 0;
    header->audioType = 0;

    for (i = 0; i < stream_mapping_size; i++) {
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;

        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_mapping[i] = -1;
            logD("Error: invalid media type %d", in_codecpar->codec_type);
            continue;
        }
        if (in_codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (video_stream_index == -1) {
                video_stream_index = i;
            } else {
                logD("Error: video_stream_index has been set to %d", video_stream_index);
            }
            if (in_codecpar->codec_id == AV_CODEC_ID_H264) {
                header->videoType = 1;
            } else if (in_codecpar->codec_id == AV_CODEC_ID_MPEG4) {
                header->videoType = 2;
            } else if (in_codecpar->codec_id == AV_CODEC_ID_JPEG2000) {
                header->videoType = 3;
            } else if (in_codecpar->codec_id == AV_CODEC_ID_MJPEG) {
                header->videoType = 4;
            } else if (in_codecpar->codec_id == AV_CODEC_ID_HEVC) {
                header->videoType = 5;
            } else {
                logD("Error: not support video type %d", in_codecpar->codec_id);
            }
            header->videoWidth = in_codecpar->width;
            header->videoHeight = in_codecpar->height;
            header->videoFrameRate = (int) (
                    in_stream->r_frame_rate.num / in_stream->r_frame_rate.den + 0.5f);
            AVDictionaryEntry *tag = NULL;
            tag = av_dict_get(in_stream->metadata, "rotate", tag, AV_DICT_IGNORE_SUFFIX);
            if (tag != NULL) {
                header->videoRotate = atoi(tag->value);
            } else {
                header->videoRotate = 0;
            }
            logD("r_frame_rate = %d|%d, rotate = %d", in_stream->r_frame_rate.num,
                 in_stream->r_frame_rate.den, header->videoRotate);
        } else if (in_codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audio_stream_index == -1) {
                audio_stream_index = i;
            } else {
                logD("Error: audio_stream_index has been set to %d", audio_stream_index);
            }
            header->audioType = 101;
            header->audioProfile = in_codecpar->profile;
            header->audioMode = (int) in_codecpar->channel_layout;
            header->audioChannels = in_codecpar->channels;
            header->audioBitWidth = in_codecpar->format;
            header->audioSampleRate = in_codecpar->sample_rate;
            header->sampleNumPerFrame = in_codecpar->frame_size;
        } else if (in_codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
            logD("Error: subtitle media type");
        }

        stream_mapping[i] = stream_index++;
    }

    logD("audio_stream_index = %d, video_stream_index = %d, stream_mapping_size = %d\n",
         audio_stream_index, video_stream_index, stream_mapping_size);

    logD("av_init\n");
    media_callback.av_init(channelId, header);
    //send SPS PPS
    media_callback.av_feed_video(channelId,
                                 ifmt_ctx->streams[video_stream_index]->codecpar->extradata,
                                 (uint32_t) ifmt_ctx->streams[video_stream_index]->codecpar->extradata_size,
                                 0, 0, 2);

    media_callback.av_feed_audio(channelId,
                                 ifmt_ctx->streams[audio_stream_index]->codecpar->extradata,
                                 (uint32_t) ifmt_ctx->streams[audio_stream_index]->codecpar->extradata_size,
                                 0, 0, 2);

    while (loop()) {
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
            AVRational time_base = ifmt_ctx->streams[(&pkt)->stream_index]->time_base;
            media_callback.av_feed_audio(channelId, pkt.data, pkt.size,
                                         pts2timeus(time_base, pkt.pts),
                                         pts2timeus(time_base, pkt.dts), 0);
        } else if (pkt.stream_index == video_stream_index) {
            print_packet(ifmt_ctx, &pkt, "video");
            AVRational time_base = ifmt_ctx->streams[(&pkt)->stream_index]->time_base;
            media_callback.av_feed_video(channelId, pkt.data, pkt.size,
                                         pts2timeus(time_base, pkt.pts),
                                         pts2timeus(time_base, pkt.dts),
                                         (pkt.flags & AV_PKT_FLAG_KEY));
        }

        if (ret < 0) {
            logD("Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);

        if (header->videoFrameRate <= 0 || header->audioSampleRate <= 0 ||
            header->sampleNumPerFrame <= 0) {
            continue;
        }
    }

    logD("av_destroy\n");
    media_callback.av_destroy(channelId);

    end:

    avformat_close_input(&ifmt_ctx);

    av_freep(&stream_mapping);

    if (ret < 0 && ret != AVERROR_EOF) {
        logD("Error occurred: %s\n", av_err2str(ret));
    }
    logD("ending");
}

int create_stream_channel(const char *url, const struct MediaCallback callback) {
    web_url = (char *) malloc(strlen(url) + 1);
    memcpy(web_url, url, strlen(url));
    web_url[strlen(url)] = '\0';
    stream_media_callback = callback;
    streamingFrame = true;
    int ret = pthread_create(&stream_thread_id, NULL, (void *) start_stream_demuxing, NULL);
    if (ret != 0) {
        return -1;
    }
    streamChannelId = 0;
    return streamChannelId;
}

int destroy_stream_channel(int channelId) {
    if (web_url) {
        free(web_url);
    }
    streamingFrame = false;
    pthread_join(stream_thread_id, NULL);
    return 0;
}

void start_stream_demuxing() {
    start_demuxing(web_url, stream_media_callback, streamChannelId, &stream_loop);
}

int is_stream_demuxing(int channelId) {
    return streamingFrame;
}

int stream_loop() {
    return streamingFrame;
}
