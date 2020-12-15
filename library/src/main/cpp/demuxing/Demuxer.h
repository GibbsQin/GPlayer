//
// Created by qinshenghua on 2020/12/15.
//

#ifndef GPLAYER_DEMUXER_H
#define GPLAYER_DEMUXER_H

extern "C" {
#include "avformat_def.h"
#include "adtsenc.h"
}

class Demuxer {
public:
    Demuxer();

    ~Demuxer();

    void start(char *filename, int channelId, FfmpegCallback callback, FormatInfo *formatInfo);
};


#endif //GPLAYER_DEMUXER_H
