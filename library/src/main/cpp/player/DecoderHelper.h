/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#ifndef GPLAYER_DECODERHELPER_H
#define GPLAYER_DECODERHELPER_H


#include <cstdint>
#include <codec/Codec.h>
#include <mutex>
#include <source/PacketSource.h>
#include <source/FrameSource.h>
#include <source/MessageSource.h>

class DecoderHelper {
public:
    DecoderHelper(PacketSource *inputSource, FrameSource *outputSource, MessageSource *messageSource);

    ~DecoderHelper();

    int onInit();

    int processAudioBuffer(int type, long extra);

    int processVideoBuffer(int type, long extra);

    void onRelease();

    void reset();

    void setMediaCodec(bool enable) {
        mediaCodecFirst = enable;
    }

    void setANativeWindow(ANativeWindow *window) {
        nativeWindow = window;
    }

    void setStopWhenEmpty(bool enable) {
        stopWhenEmpty = enable;
    }

private:
    bool hasInit;
    bool mediaCodecFirst;
    bool stopWhenEmpty = false;
    PacketSource *inputSource;
    FrameSource *outputSource;
    MessageSource *messageSource;
    VideoDecoder *videoDecoder{};
    AudioDecoder *audioDecoder{};
    MediaData *videoOutFrame{};
    MediaData *audioOutFrame{};
    std::mutex audioLock;
    std::mutex videoLock;
    ANativeWindow  *nativeWindow{};
};


#endif //GPLAYER_DECODERHELPER_H
