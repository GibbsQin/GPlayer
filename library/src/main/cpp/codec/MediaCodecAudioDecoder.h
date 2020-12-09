//
// Created by Gibbs on 2020/7/21.
//

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

    void init(AVCodecParameters *codecParameters);

    int send_packet(AVPacket *inPacket);

    int receive_frame(MediaData *outFrame);

    void extractFrame(uint8_t *outputBuf, MediaData *outFrame, AMediaCodecBufferInfo info);

    void release();

protected:
    AMediaCodec *mAMediaCodec{};
};


#endif //GPLAYER_MEDIACODECAUDIODECODER_H
