/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#ifndef GPLAYER_PACKETSOURCE_H
#define GPLAYER_PACKETSOURCE_H


#include "MessageSource.h"
#include "ConcurrentQueue.h"

extern "C" {
#include <demuxing/avformat_def.h>
#include "codec/ffmpeg/libavcodec/avcodec.h"
}

class PacketSource {

public:
    PacketSource(int audioMaxSize, int videoMaxSize);

    ~PacketSource();

public:
    void setFormatInfo(FormatInfo *formatInfo);

    FormatInfo* getFormatInfo();

    AVCodecParameters* getAudioAVCodecParameters();

    AVCodecParameters* getVideoAVCodecParameters();

    unsigned long pushAudPkt(AVPacket *pkt, AVRational time_base);

    unsigned long pushVidPkt(AVPacket *pkt, AVRational time_base);

    unsigned long readAudPkt(AVPacket **pkt);

    unsigned long readVidPkt(AVPacket **pkt);

    void popAudPkt(AVPacket *pkt);

    void popVidPkt(AVPacket *pkt);

    void flush();

    void reset();

    unsigned long audioSize();

    unsigned long videoSize();

private:
    FormatInfo *mFormatInfo;
    AvConcurrentQueue *videoPacketQueue;
    AvConcurrentQueue *audioPacketQueue;
};


#endif //GPLAYER_PACKETSOURCE_H
