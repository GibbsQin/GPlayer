/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#ifndef GPLAYER_DEMUXERHELPER_H
#define GPLAYER_DEMUXERHELPER_H

#include <source/PacketSource.h>
#include <source/MessageSource.h>

extern "C" {
#include "adtsenc.h"
#include <codec/ffmpeg/libavutil/timestamp.h>
}

#define HAS_AUDIO 0x1
#define HAS_VIDEO 0x2

class DemuxerHelper {
public:
    DemuxerHelper(const std::string &url, PacketSource *input, MessageSource *messageSource);

    ~DemuxerHelper();

    void init();

    int readPacket(int type, long extra);

    void release();

    void notifyError(int ret);

    bool needDiscardPkt(AVPacket pkt);

private:
    char *filename;
    PacketSource *inputSource;
    MessageSource *messageSource;

    AVFormatContext *ifmt_ctx = nullptr;
    int audio_stream_index = -1;
    int video_stream_index = -1;
    int subtitle_stream_index = -1;
    int64_t seekFrameUs = -1;
    bool errorExist = false;

    int needVideoStreamFilter = 0;
    int needAudioStreamFilter = 0;
    ADTSContext mADTSContext{};
    unsigned char mADTSHeader[ADTS_HEADER_SIZE] = {0};
    AVBSFContext *video_abs_ctx = nullptr;
    const AVBitStreamFilter *video_abs_filter = nullptr;
};


#endif //GPLAYER_DEMUXERHELPER_H
