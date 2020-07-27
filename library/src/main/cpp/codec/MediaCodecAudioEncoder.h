//
// Created by Gibbs on 2020/7/21.
//

#ifndef GPLAYER_MEDIACODECAUDIOENCODER_H
#define GPLAYER_MEDIACODECAUDIOENCODER_H


#include <cstdint>
#include "Codec.h"
#include <media/NdkMediaCodec.h>

class MediaCodecAudioEncoder : public AudioEncoder {
public:

    MediaCodecAudioEncoder();

    ~MediaCodecAudioEncoder();

    void init(MediaInfo *header);

    int send_frame(MediaData *inFrame);

    int receive_packet(MediaData *outPacket);

    void release();

private:
    MediaInfo *mHeader{};
    AMediaCodec *mAMediaCodec{};
};


#endif //GPLAYER_MEDIACODECAUDIOENCODER_H
