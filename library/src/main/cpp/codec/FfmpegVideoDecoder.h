/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#ifndef GPLAYER_FFMPEGVIDEODECODER_H
#define GPLAYER_FFMPEGVIDEODECODER_H

extern "C" {
#include <ffmpeg/libavcodec/avcodec.h>
#include <ffmpeg/libavformat/avformat.h>
}

#include "Codec.h"

class FfmpegVideoDecoder : public VideoDecoder {
public:

    FfmpegVideoDecoder();

    ~FfmpegVideoDecoder();

    void init(AVCodecParameters *codecParameters) override;

    int send_packet(AVPacket *inPacket) override;

    int receive_frame(MediaData *outFrame) override;

    void release_buffer() override;

    void release() override;

    void reset() override;

    void setNativeWindow(ANativeWindow *nativeWindow) override;

    static void copy_mediadata_from_frame(MediaData *mediaData, AVFrame *frame);

private:
    bool isInitSuccess = false;
    AVCodecContext *mCodecContext = nullptr;
    AVFrame *mOutFrame = nullptr;
};


#endif //GPLAYER_FFMPEGVIDEODECODER_H
