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
    void queueInfo(FormatInfo *formatInfo);

    uint32_t queueAudPkt(AVPacket *pkt);

    uint32_t queueVidPkt(AVPacket *pkt);

public:
    FormatInfo* getFormatInfo();

    AVCodecParameters* getAudioAVCodecParameters();

    AVCodecParameters* getVideoAVCodecParameters();

    int dequeAudPkt(AVPacket **pkt);

    int dequeVidPkt(AVPacket **pkt);

    void popAudPkt();

    void popVidPkt();

    void flush();

    uint32_t getAudSize();

    uint32_t getVidSize();

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
