#ifndef GPLAYER_MEDIASOURCE_H
#define GPLAYER_MEDIASOURCE_H


#include <jni.h>
#include <string>
#include <queue>
#include "Codec.h"
#include "player/DecodeThread.h"

#define AV_SOURCE_EMPTY -2

#define AV_FLAG_SOURCE_MEDIA_CODEC 0x00000002

class MediaSource {

public:

    MediaSource();

    ~MediaSource();

    MediaInfo* getAVHeader();

    void onInit(MediaInfo *header);

    uint32_t onReceiveAudio(MediaData *inPacket);

    uint32_t onReceiveVideo(MediaData *inPacket);

    void onRelease();

    void flushBuffer();

    void flushAudioBuffer();

    void flushVideoBuffer();

    int readAudioBuffer(MediaData** avData);

    int readVideoBuffer(MediaData** avData);

    void popAudioBuffer();

    void popVideoBuffer();

    uint32_t getAudioBufferSize();

    uint32_t getVideoBufferSize();

private:
    MediaInfo *mAVHeader;
    std::queue<MediaData *> videoPacketQueue;
    std::queue<MediaData *> audioPacketQueue;
    std::mutex mAudioLock;
    std::mutex mVideoLock;
};


#endif //GPLAYER_MEDIASOURCE_H
