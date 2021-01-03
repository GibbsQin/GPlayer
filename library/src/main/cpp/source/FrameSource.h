/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#ifndef GPLAYER_FRAMESOURCE_H
#define GPLAYER_FRAMESOURCE_H


#include "media/MediaData.h"
#include "ConcurrentQueue.h"

extern "C" {
#include <demuxing/avformat_def.h>
}

class FrameSource {

public:
    FrameSource(int audioMaxSize, int videoMaxSize);

    ~FrameSource();

public:
    unsigned long pushAudFrame(MediaData *frame);

    unsigned long pushVidFrame(MediaData *frame);

    unsigned long readAudFrame(MediaData **frame);

    unsigned long readVidFrame(MediaData **frame);

    void popAudFrame(MediaData *frame);

    void popVidFrame(MediaData *frame);

    void flush();

    void reset();

    unsigned long audioSize();

    unsigned long videoSize();

private:
    ConcurrentQueue<MediaData *> *audioPacketQueue;
    ConcurrentQueue<MediaData *> *videoPacketQueue;
};


#endif //GPLAYER_FRAMESOURCE_H
