#ifndef GPLAYER_INPUTSOURCE_H
#define GPLAYER_INPUTSOURCE_H


#include <jni.h>
#include <string>
#include <deque>
#include <mutex>

extern "C" {
#include <demuxing/avformat_def.h>
#include "../codec/ffmpeg/libavcodec/avcodec.h"
}

#define AV_SOURCE_EMPTY -2

class InputSource {

public:
    InputSource();

    ~InputSource();

public:
    void onInit(FormatInfo *formatInfo);

    uint32_t onReceiveAudio(AVPacket *pkt);

    uint32_t onReceiveVideo(AVPacket *pkt);

    void onRelease();

public:
    FormatInfo* getFormatInfo();

    AVCodecParameters* getAudioAVCodecParameters();

    AVCodecParameters* getVideoAVCodecParameters();

    AVCodecParameters* getSubtitleAVCodecParameters();

    int readAudioBuffer(AVPacket **pkt);

    int readVideoBuffer(AVPacket **pkt);

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
    std::deque<AVPacket*> videoPacketQueue;
    std::deque<AVPacket*> audioPacketQueue;
    std::mutex mAudioLock;
    std::mutex mVideoLock;
};


#endif //GPLAYER_INPUTSOURCE_H
