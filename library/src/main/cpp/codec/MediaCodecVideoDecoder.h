/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#ifndef GPLAYER_MEDIACODECVIDEODECODER_H
#define GPLAYER_MEDIACODECVIDEODECODER_H


#include <cstdint>
#include "Codec.h"
#include <media/NdkMediaCodec.h>
#include <android/native_window.h>

class MediaCodecVideoDecoder : public VideoDecoder {
public:

    MediaCodecVideoDecoder();

    ~MediaCodecVideoDecoder();

    void init(AVCodecParameters *codecParameters) override;

    int send_packet(AVPacket *inPacket) override;

    int receive_frame(MediaData *outFrame) override;

    void release_buffer() override;

    void release() override;

    void reset() override;

    void setNativeWindow(ANativeWindow *window) override {
        this->nativeWindow = window;
    }

protected:
    AMediaCodec *mAMediaCodec;
    ANativeWindow *nativeWindow;
    int mWidth = 0;
    int mHeight = 0;
    ssize_t mCurrentBufferId = -1;
};


#endif //GPLAYER_MEDIACODECVIDEODECODER_H
