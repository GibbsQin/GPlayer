#ifndef GPLAYER_REMUXING_H
#define GPLAYER_REMUXING_H

#include <codec/ffmpeg/libavcodec/avcodec.h>
#include <codec/ffmpeg/libavformat/avformat.h>
#include <media/Media.h>

#define MAX_AV_TIME 2

#define SLEEP_TIME_US 100000

#define MAX_FRAME_RATE 24

typedef int (*ffmpeg_loop_wait)();

typedef struct FfmpegCallback {
    void (*av_format_init_audio)(AVFormatContext *ifmt_ctx, AVStream *stream);

    void (*av_format_init_video)(AVFormatContext *ifmt_ctx, AVStream *stream);

    void (*av_format_extradata_audio)(AVFormatContext *ifmt_ctx, uint8_t *pInputBuf, uint32_t dwInputDataSize);

    void (*av_format_extradata_video)(AVFormatContext *ifmt_ctx, uint8_t *pInputBuf, uint32_t dwInputDataSize);

    uint32_t (*av_format_feed_audio)(AVFormatContext *ifmt_ctx, AVPacket *packet);

    uint32_t (*av_format_feed_video)(AVFormatContext *ifmt_ctx, AVPacket *packet);

    void (*av_format_destroy)(AVFormatContext *ifmt_ctx);
} FfmpegCallback;

uint64_t ffmpeg_pts2timeus(AVRational time_base, int64_t pts);

void ffmpeg_demuxing(const char *in_filename, const struct FfmpegCallback media_callback,
                     ffmpeg_loop_wait loop);

void ffmpeg_extra_audio_info(AVFormatContext *ifmt_ctx, AVStream *stream, MediaInfo *mediaInfo);

void ffmpeg_extra_video_info(AVFormatContext *ifmt_ctx, AVStream *stream, MediaInfo *mediaInfo);

#endif //GPLAYER_REMUXING_H
