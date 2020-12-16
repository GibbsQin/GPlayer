//
// Created by Gibbs on 2020/7/18.
//

#ifndef GPLAYER_DECODEHELPER_H
#define GPLAYER_DECODEHELPER_H


#include <cstdint>
#include <codec/Codec.h>
#include <mutex>

#define AV_TYPE_AUDIO 0
#define AV_TYPE_VIDEO 1

class DecodeHelper {
public:
    DecodeHelper(bool mediaCodecFirst);

    ~DecodeHelper();

    int onInit(FormatInfo *formatInfo);

    int inputBuffer(AVPacket *buffer, int type);

    int outputBuffer(MediaData **buffer, int type);

    void onRelease();

    void enableMediaCodec();

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


#endif //GPLAYER_DECODEHELPER_H
