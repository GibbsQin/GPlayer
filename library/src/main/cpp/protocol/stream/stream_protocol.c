#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <pthread.h>
#include <stdbool.h>
#include <malloc.h>
#include <android/log.h>
#include <media/Media.h>
#include <codec/ffmpeg/libavutil/timestamp.h>
#include <protocol/remuxing.h>
#include <unistd.h>
#include "stream_protocol.h"
#include "../codec/ffmpeg/libavutil/rational.h"
#include "../codec/ffmpeg/libavformat/avformat.h"
#include "../codec/ffmpeg/libavcodec/avcodec.h"
#include "../codec/ffmpeg/libavutil/avutil.h"

static void av_format_init(AVFormatContext *ifmt_ctx, AVStream *audioStream, AVStream *videoStream) {
    ffmpeg_extra_audio_info(ifmt_ctx, audioStream, mediaInfo);
    ffmpeg_extra_video_info(ifmt_ctx, videoStream, mediaInfo);
    stream_media_callback.av_init(streamChannelId, mediaInfo);
}

static void av_format_extradata_audio(AVFormatContext *ifmt_ctx, uint8_t *pInputBuf, uint32_t dwInputDataSize) {
    stream_media_callback.av_feed_audio(streamChannelId, pInputBuf, dwInputDataSize, 0, 0, 2);
}

static void av_format_extradata_video(AVFormatContext *ifmt_ctx, uint8_t *pInputBuf, uint32_t dwInputDataSize) {
    stream_media_callback.av_feed_video(streamChannelId, pInputBuf, dwInputDataSize, 0, 0, 2);
}

static uint32_t av_format_feed_audio(AVFormatContext *ifmt_ctx, AVPacket *packet) {
    AVRational time_base = ifmt_ctx->streams[packet->stream_index]->time_base;
    return stream_media_callback.av_feed_audio(streamChannelId, packet->data, packet->size,
                                               ffmpeg_pts2timeus(time_base, packet->pts),
                                               ffmpeg_pts2timeus(time_base, packet->dts), 0);
}

static uint32_t av_format_feed_video(AVFormatContext *ifmt_ctx, AVPacket *packet) {
    AVRational time_base = ifmt_ctx->streams[packet->stream_index]->time_base;
    int result = stream_media_callback.av_feed_video(streamChannelId, packet->data, packet->size,
                                               ffmpeg_pts2timeus(time_base, packet->pts),
                                               ffmpeg_pts2timeus(time_base, packet->dts),
                                               (packet->flags & AV_PKT_FLAG_KEY));
    if (result > MAX_FRAME_RATE * MAX_AV_TIME) {
        streamSleepTimeUs += SLEEP_TIME_US;
    } else {
        streamSleepTimeUs = 0;
    }
    if (streamSleepTimeUs > 0) {
        usleep((useconds_t) streamSleepTimeUs);
        streamSleepTimeUs = 0;
    }
    return result;
}

static void av_format_destroy(AVFormatContext *ifmt_ctx) {
    stream_media_callback.av_destroy(streamChannelId);
}

static void av_format_error(int code, char *msg) {
    stream_media_callback.av_error(streamChannelId, code, msg);
}

int create_stream_channel(const char *url, const struct MediaCallback callback) {
    web_url = (char *) malloc(strlen(url) + 1);
    memcpy(web_url, url, strlen(url));
    web_url[strlen(url)] = '\0';
    stream_media_callback = callback;
    ffmpeg_callback.av_format_init = av_format_init;
    ffmpeg_callback.av_format_extradata_audio = av_format_extradata_audio;
    ffmpeg_callback.av_format_extradata_video = av_format_extradata_video;
    ffmpeg_callback.av_format_feed_audio = av_format_feed_audio;
    ffmpeg_callback.av_format_feed_video = av_format_feed_video;
    ffmpeg_callback.av_format_destroy = av_format_destroy;
    ffmpeg_callback.av_format_error = av_format_error;
    streamingFrame = true;
    mediaInfo = malloc(sizeof(MediaInfo));
    mediaInfo->audioType = 0;
    mediaInfo->videoType = 0;
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
    free(mediaInfo);
    mediaInfo = NULL;
    return 0;
}

void start_stream_demuxing() {
    ffmpeg_demuxing(web_url, ffmpeg_callback, &stream_loop);
}

int is_stream_demuxing(int channelId) {
    return streamingFrame;
}

int stream_loop() {
    return streamingFrame;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */