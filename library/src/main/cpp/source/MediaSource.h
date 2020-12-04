#ifndef GPLAYER_MEDIASOURCE_H
#define GPLAYER_MEDIASOURCE_H


#include <jni.h>
#include <string>
#include <queue>
#include <mutex>
#include "media/MediaData.h"
extern "C" {
#include <demuxing/avformat_def.h>
}

#define AV_SOURCE_EMPTY -2

class MediaSource {

public:
    MediaSource();

    ~MediaSource();

public:
    void onInit(MediaInfo *header);

    uint32_t onReceiveAudio(MediaData *inPacket);

    uint32_t onReceiveVideo(MediaData *inPacket);

    void onRelease();

public:
    MediaInfo *getAVHeader();

    int readAudioBuffer(MediaData **avData);

    int readVideoBuffer(MediaData **avData);

    void popAudioBuffer();

    void popVideoBuffer();

    void flushBuffer();

    uint32_t getAudioBufferSize();

    uint32_t getVideoBufferSize();

private:
    void flushAudioBuffer();

    void flushVideoBuffer();

private:
    MediaInfo *mAVHeader;
    std::queue<MediaData *> videoPacketQueue;
    std::queue<MediaData *> audioPacketQueue;
    std::mutex mAudioLock;
    std::mutex mVideoLock;
};


#endif //GPLAYER_MEDIASOURCE_H
