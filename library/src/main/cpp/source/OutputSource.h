#ifndef GPLAYER_OUTPUTSOURCE_H
#define GPLAYER_OUTPUTSOURCE_H


#include <jni.h>
#include <string>
#include <deque>
#include <mutex>
#include "media/MediaData.h"
extern "C" {
#include <demuxing/avformat_def.h>
}

#define AV_SOURCE_EMPTY -2

class OutputSource {

public:
    OutputSource();

    ~OutputSource();

public:
    void onInit(FormatInfo *formatInfo);

    uint32_t onReceiveAudio(MediaData *inPacket);

    uint32_t onReceiveVideo(MediaData *inPacket);

    void onRelease();

public:
    FormatInfo* getFormatInfo();

    AVCodecParameters* getAudioAVCodecParameters();

    AVCodecParameters* getVideoAVCodecParameters();

    AVCodecParameters* getSubtitleAVCodecParameters();

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
    FormatInfo *mFormatInfo;
    std::deque<MediaData *> videoPacketQueue;
    std::deque<MediaData *> audioPacketQueue;
    std::mutex mAudioLock;
    std::mutex mVideoLock;
};


#endif //GPLAYER_OUTPUTSOURCE_H
