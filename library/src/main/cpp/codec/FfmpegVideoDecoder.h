#ifndef GPLAYER_FFMPEGVIDEODECODER_H
#define GPLAYER_FFMPEGVIDEODECODER_H

extern "C"
{
#include <ffmpeg/libavcodec/avcodec.h>
#include <ffmpeg/libavformat/avformat.h>
}

#include "Codec.h"

#define TAG "FfmpegVideoDecoder"

class FfmpegVideoDecoder : public VideoDecoder {
public:

    FfmpegVideoDecoder();

    ~FfmpegVideoDecoder();

    virtual void init(AVCodecParameters *codecParameters) override;

    virtual int send_packet(MediaData *inPacket) override;

    virtual int receive_frame(MediaData *outFrame) override;

    virtual void release() override;

private:
    bool isInitSuccess;
    AVCodec *mCodec;
    AVCodecContext *mCodecContext;
    AVFrame *mOutFrame;
    AVPacket *mInPacket;
};


#endif //GPLAYER_FFMPEGVIDEODECODER_H
