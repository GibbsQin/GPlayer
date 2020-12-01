#ifndef GPLAYER_MEDIAPIPE_H
#define GPLAYER_MEDIAPIPE_H

#include <stdint.h>
#include "MediaSource.h"
#include "GPlayerEngine.h"
#include <map>

extern "C" {
#include <codec/ffmpeg/libavformat/avformat.h>
#include <protocol/remuxing.h>
}

class MediaPipe {
public:
    static void av_format_init(int channel, AVFormatContext *ifmt_ctx,
                               AVStream *audioStream, AVStream *videoStream, MediaInfo *mediaInfo);

    static void av_format_extradata_audio(int channel, AVFormatContext *ifmt_ctx,
                                          uint8_t *pInputBuf, uint32_t dwInputDataSize);

    static void av_format_extradata_video(int channel, AVFormatContext *ifmt_ctx,
                                          uint8_t *pInputBuf, uint32_t dwInputDataSize);

    static uint32_t av_format_feed_audio(int channel, AVFormatContext *ifmt_ctx, AVPacket *packet);

    static uint32_t av_format_feed_video(int channel, AVFormatContext *ifmt_ctx, AVPacket *packet);

    static void av_format_destroy(int channel, AVFormatContext *ifmt_ctx);

    static void av_format_error(int channel, int code, char *msg);

    static uint32_t av_format_loop_wait(int channel);

    static void av_init(int channelId, MediaInfo *header);

    static uint32_t av_feed_audio(int channelId, uint8_t *pInputBuf, uint32_t dwInputDataSize,
                                  uint64_t u64InputPTS, uint64_t u64InputDTS, int flag);

    static uint32_t av_feed_video(int channelId, uint8_t *pInputBuf, uint32_t dwInputDataSize,
                                  uint64_t u64InputPTS, uint64_t u64InputDTS, int flag);

    static void av_destroy(int channelId);

    static void av_error(int channelId, int code, char *msg);

    static std::map<long, GPlayerEngine *> sGPlayerMap;

    static void deleteFromMap(int channelId);

    static FfmpegCallback sFfmpegCallback;
};

#endif //GPLAYER_MEDIAPIPE_H
