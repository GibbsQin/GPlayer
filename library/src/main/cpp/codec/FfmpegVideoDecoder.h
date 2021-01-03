#ifndef GPLAYER_FFMPEGVIDEODECODER_H
#define GPLAYER_FFMPEGVIDEODECODER_H

extern "C"
{
#include <ffmpeg/libavcodec/avcodec.h>
#include <ffmpeg/libavformat/avformat.h>
}

#include "Codec.h"

class FfmpegVideoDecoder : public VideoDecoder {
public:

    FfmpegVideoDecoder();

    ~FfmpegVideoDecoder();

    virtual void init(AVCodecParameters *codecParameters) override;

    virtual int send_packet(AVPacket *inPacket) override;

    virtual int receive_frame(MediaData *outFrame) override;

    virtual void release() override;

    void copy_mediadata_from_frame(MediaData *mediaData, AVFrame *frame);

private:
    bool isInitSuccess{};
    AVCodecContext *mCodecContext{};
    AVFrame *mOutFrame{};
};


#endif //GPLAYER_FFMPEGVIDEODECODER_H
