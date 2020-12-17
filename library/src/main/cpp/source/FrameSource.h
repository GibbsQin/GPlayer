#ifndef GPLAYER_FRAMESOURCE_H
#define GPLAYER_FRAMESOURCE_H


#include <jni.h>
#include <string>
#include <deque>
#include <mutex>
#include "media/MediaData.h"
extern "C" {
#include <demuxing/avformat_def.h>
}

#define AV_SOURCE_EMPTY -2

class FrameSource {

public:
    FrameSource();

    ~FrameSource();

public:
    uint32_t onReceiveAudio(MediaData *inPacket);

    uint32_t onReceiveVideo(MediaData *inPacket);

public:
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
    std::deque<MediaData *> videoPacketQueue;
    std::deque<MediaData *> audioPacketQueue;
    std::mutex mAudioLock;
    std::mutex mVideoLock;
};


#endif //GPLAYER_FRAMESOURCE_H
