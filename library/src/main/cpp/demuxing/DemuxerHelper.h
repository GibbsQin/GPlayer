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
    DemuxerHelper(char *url, GPlayer *p, FormatInfo *info);

    void init();

    int update();

    void release();

    void notifyError(int ret);

private:
    char *filename;
    GPlayer *player;
    FormatInfo *formatInfo;

    AVFormatContext *ifmt_ctx = nullptr;
    int audio_stream_index = -1;
    int video_stream_index = -1;
    int subtitle_stream_index = -1;

    int needVideoStreamFilter = 0;
    int needAudioStreamFilter = 0;
    ADTSContext mADTSContext;
    unsigned char mADTSHeader[ADTS_HEADER_SIZE] = {0};
    AVBSFContext *video_abs_ctx = nullptr;
    const AVBitStreamFilter *video_abs_filter = nullptr;
};


#endif //GPLAYER_DEMUXERHELPER_H
