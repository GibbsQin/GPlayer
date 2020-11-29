#ifndef GPLAYER_REMUXING_H
#define GPLAYER_REMUXING_H

#include <codec/ffmpeg/libavcodec/avcodec.h>
#include <codec/ffmpeg/libavformat/avformat.h>
#include <media/Media.h>

#define MAX_AV_TIME 2

#define SLEEP_TIME_US 100000

#define MAX_FRAME_RATE 24

typedef struct FfmpegCallback {
    void (*av_format_init)(int channel, AVFormatContext *ifmt_ctx, AVStream *audioStream,
                           AVStream *videoStream, MediaInfo *mediaInfo);

    void (*av_format_extradata_audio)(int channel, AVFormatContext *ifmt_ctx, uint8_t *pInputBuf,
                                      uint32_t dwInputDataSize);

    void (*av_format_extradata_video)(int channel, AVFormatContext *ifmt_ctx, uint8_t *pInputBuf,
                                      uint32_t dwInputDataSize);

    uint32_t (*av_format_feed_audio)(int channel, AVFormatContext *ifmt_ctx, AVPacket *packet);

    uint32_t (*av_format_feed_video)(int channel, AVFormatContext *ifmt_ctx, AVPacket *packet);

    void (*av_format_destroy)(int channel, AVFormatContext *ifmt_ctx);

    void (*av_format_error)(int channel, int code, char *msg);

    uint32_t (*av_format_loop_wait)(int channel);
} FfmpegCallback;

typedef struct DemuxingParams {
    const char *filename;
    FfmpegCallback callback;
    MediaInfo *mediaInfo;
    const int *channelId;
} DemuxingParams;

uint64_t ffmpeg_pts2timeus(AVRational time_base, int64_t pts);

void ffmpeg_demuxing(void *data);

void ffmpeg_extra_audio_info(AVFormatContext *ifmt_ctx, AVStream *stream, MediaInfo *mediaInfo);

void ffmpeg_extra_video_info(AVFormatContext *ifmt_ctx, AVStream *stream, MediaInfo *mediaInfo);

#endif //GPLAYER_REMUXING_H
