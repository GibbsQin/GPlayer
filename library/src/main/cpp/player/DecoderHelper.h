//
// Created by Gibbs on 2020/7/18.
//

#ifndef GPLAYER_DECODERHELPER_H
#define GPLAYER_DECODERHELPER_H


#include <cstdint>
#include <codec/Codec.h>
#include <mutex>
#include <source/PacketSource.h>
#include <source/FrameSource.h>
#include <source/MessageQueue.h>

#define AV_TYPE_AUDIO 0
#define AV_TYPE_VIDEO 1

class DecoderHelper {
public:
    DecoderHelper(PacketSource *inputSource, FrameSource *outputSource, MessageQueue *messageQueue);

    ~DecoderHelper();

    int onInit();

    int processAudioBuffer(int type, long extra);

    int processVideoBuffer(int type, long extra);

    int inputBuffer(AVPacket *buffer, int type);

    int outputBuffer(MediaData **buffer, int type);

    void onRelease();

    void setMediaCodec(bool enable) {
        mediaCodecFirst = enable;
    }

private:
    bool hasInit;
    bool mediaCodecFirst;
    PacketSource *inputSource;
    FrameSource *outputSource;
    MessageQueue *messageQueue;
    VideoDecoder *videoDecoder{};
    AudioDecoder *audioDecoder{};
    MediaData *videoOutFrame{};
    MediaData *audioOutFrame{};
    std::mutex audioLock;
    std::mutex videoLock;
};


#endif //GPLAYER_DECODERHELPER_H
