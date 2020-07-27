//
// Created by Gibbs on 2020/7/21.
//

#ifndef GPLAYER_MEDIACODECAUDIODECODER_H
#define GPLAYER_MEDIACODECAUDIODECODER_H


#include "MediaCodecVideoDecoder.h"

class MediaCodecAudioDecoder : public AudioDecoder {
public:

    MediaCodecAudioDecoder();

    ~MediaCodecAudioDecoder();

    void init(MediaInfo *header);

    int send_packet(MediaData *inPacket);

    int receive_frame(MediaData *outFrame);

    void extractFrame(uint8_t *outputBuf, MediaData *outFrame, AMediaCodecBufferInfo info);

    void release();

protected:
    MediaInfo *mHeader{};
    AMediaCodec *mAMediaCodec{};
};


#endif //GPLAYER_MEDIACODECAUDIODECODER_H
