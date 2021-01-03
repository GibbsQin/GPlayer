/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#ifndef GPLAYER_FFMPEGAUDIODECODER_H
#define GPLAYER_FFMPEGAUDIODECODER_H

#include "Codec.h"

extern "C" {
#include <ffmpeg/libavcodec/avcodec.h>
#include <ffmpeg/libavformat/avformat.h>
#include <ffmpeg/libswresample/swresample.h>
}

class FfmpegAudioDecoder : public AudioDecoder {
public:
    FfmpegAudioDecoder();

    ~FfmpegAudioDecoder();

    void init(AVCodecParameters *codecParameters) override;

    int send_packet(AVPacket *inPacket) override;

    int receive_frame(MediaData *outFrame) override;

    void release() override;

    void reset() override;

private:
    bool isInitSuccess = false;
    AVCodecContext *mCodecContext = nullptr;
    AVFrame *mOutFrame = nullptr;
};


#endif //GPLAYER_FFMPEGAUDIODECODER_H
