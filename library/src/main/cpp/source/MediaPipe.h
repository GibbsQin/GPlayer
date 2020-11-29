#ifndef GPLAYER_MEDIAPIPE_H
#define GPLAYER_MEDIAPIPE_H

#include <stdint.h>
#include "MediaSource.h"
#include "GPlayerEngine.h"
#include <map>

class MediaPipe {
public:
    static void av_init(int channelId, MediaInfo *header);

    static uint32_t av_feed_audio(int channelId, uint8_t *pInputBuf, uint32_t dwInputDataSize,
                                  uint64_t u64InputPTS, uint64_t u64InputDTS, int flag);

    static uint32_t av_feed_video(int channelId, uint8_t *pInputBuf, uint32_t dwInputDataSize,
                                  uint64_t u64InputPTS, uint64_t u64InputDTS, int flag);

    static void av_destroy(int channelId);

    static void av_error(int channelId, int code, char *msg);

    static std::map<long, GPlayerEngine *> sGPlayerMap;

    static void deleteFromMap(int channelId);
};

#endif //GPLAYER_MEDIAPIPE_H
