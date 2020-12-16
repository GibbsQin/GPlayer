//
// Created by qinshenghua on 2020/12/15.
//

#ifndef GPLAYER_DEMUXERHELPER_H
#define GPLAYER_DEMUXERHELPER_H

#include <player/GPlayer.h>

extern "C" {
#include "adtsenc.h"
#include <codec/ffmpeg/libavutil/timestamp.h>
}

class DemuxerHelper {
public:
    void start(char *filename, GPlayer *player, FormatInfo *formatInfo);
};


#endif //GPLAYER_DEMUXERHELPER_H
