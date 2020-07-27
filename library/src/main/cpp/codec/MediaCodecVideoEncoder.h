//
// Created by Gibbs on 2020/7/21.
//

#ifndef GPLAYER_MEDIACODECVIDEOENCODER_H
#define GPLAYER_MEDIACODECVIDEOENCODER_H


#include <cstdint>
#include "Codec.h"
#include <media/NdkMediaCodec.h>

class MediaCodecVideoEncoder : public VideoEncoder {
public:

    MediaCodecVideoEncoder();

    ~MediaCodecVideoEncoder();

    void init(MediaInfo *header);

    int send_frame(MediaData *inFrame);

    int receive_packet(MediaData *outPacket);

    void release();

private:
    MediaInfo *mHeader{};
    AMediaCodec *mAMediaCodec{};
};


#endif //GPLAYER_MEDIACODECVIDEOENCODER_H
