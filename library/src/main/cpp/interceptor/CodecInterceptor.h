//
// Created by Gibbs on 2020/7/18.
//

#ifndef GPLAYER_CODECINTERCEPTOR_H
#define GPLAYER_CODECINTERCEPTOR_H


#include <cstdint>
#include <codec/Codec.h>
#include <mutex>
#include "Interceptor.h"

class CodecInterceptor : public Interceptor {
public:
    CodecInterceptor(bool mediaCodecFirst);

    ~CodecInterceptor();

    int onInit(FormatInfo *formatInfo) override;

    int inputBuffer(AVPacket *buffer, int type) override;

    int outputBuffer(MediaData **buffer, int type) override;

    void onRelease() override;

    void enableMediaCodec() override;

private:
    bool hasInit;
    bool mediaCodecFirst;
    VideoDecoder *videoDecoder{};
    AudioDecoder *audioDecoder{};
    MediaData *videoOutFrame{};
    MediaData *audioOutFrame{};
    std::mutex audioLock;
    std::mutex videoLock;
};


#endif //GPLAYER_CODECINTERCEPTOR_H
