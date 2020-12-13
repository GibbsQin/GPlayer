#ifndef GPLAYER_MEDIAPIPE_H
#define GPLAYER_MEDIAPIPE_H

#include <stdint.h>
#include <map>
#include "player/GPlayer.h"

extern "C" {
#include <codec/ffmpeg/libavformat/avformat.h>
}

class MediaPipe {
public:
    static void av_format_init(int channel, FormatInfo *formatInfo);

    static void av_format_extradata_audio(int channel, AVFormatContext *ifmt_ctx,
                                          uint8_t *pInputBuf, uint32_t dwInputDataSize);

    static void av_format_extradata_video(int channel, AVFormatContext *ifmt_ctx,
                                          uint8_t *pInputBuf, uint32_t dwInputDataSize);

    static uint32_t av_format_feed_audio(int channel, AVFormatContext *ifmt_ctx, AVPacket *packet);

    static uint32_t av_format_feed_video(int channel, AVFormatContext *ifmt_ctx, AVPacket *packet);

    static void av_format_destroy(int channel, AVFormatContext *ifmt_ctx);

    static void av_format_error(int channel, int code, char *msg);

    static LoopFlag av_format_loop_wait(int channel, int64_t *seekUs);

    static std::map<long, GPlayer *> sGPlayerMap;

    static void deleteFromMap(int channelId);

    static FfmpegCallback sFfmpegCallback;

    static void initFfmpegCallback();
};

#endif //GPLAYER_MEDIAPIPE_H
