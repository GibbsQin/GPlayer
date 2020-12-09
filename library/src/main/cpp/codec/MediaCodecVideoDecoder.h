//
// Created by Gibbs on 2020/7/21.
//

#ifndef GPLAYER_MEDIACODECVIDEODECODER_H
#define GPLAYER_MEDIACODECVIDEODECODER_H


#include <cstdint>
#include "Codec.h"
#include <media/NdkMediaCodec.h>

class MediaCodecVideoDecoder : public VideoDecoder {
public:

    MediaCodecVideoDecoder();

    ~MediaCodecVideoDecoder();

    void init(AVCodecParameters *codecParameters);

    int send_packet(AVPacket *inPacket);

    int receive_frame(MediaData *outFrame);

    void extractFrame(uint8_t *outputBuf, MediaData *outFrame, AMediaCodecBufferInfo info);

    void release();

protected:
    AMediaCodec *mAMediaCodec{};
    int mWidth{};
    int mHeight{};
};


#endif //GPLAYER_MEDIACODECVIDEODECODER_H
