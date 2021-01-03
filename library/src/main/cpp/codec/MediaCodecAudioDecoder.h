/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#ifndef GPLAYER_MEDIACODECAUDIODECODER_H
#define GPLAYER_MEDIACODECAUDIODECODER_H


#include "MediaCodecVideoDecoder.h"

extern "C" {
#include <codec/ffmpeg/libavcodec/avcodec.h>
};

class MediaCodecAudioDecoder : public AudioDecoder {
public:

    MediaCodecAudioDecoder();

    ~MediaCodecAudioDecoder();

    void init(AVCodecParameters *codecParameters) override;

    int send_packet(AVPacket *inPacket) override;

    int receive_frame(MediaData *outFrame) override;

    void release() override;

    void reset() override;

    static void extractFrame(uint8_t *outputBuf, MediaData *outFrame, AMediaCodecBufferInfo info);

protected:
    AMediaCodec *mAMediaCodec = nullptr;
};


#endif //GPLAYER_MEDIACODECAUDIODECODER_H
