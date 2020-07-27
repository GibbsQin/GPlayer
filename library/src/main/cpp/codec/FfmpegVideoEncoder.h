#ifndef GPLAYER_FFMPEGVIDEOENCODER_H
#define GPLAYER_FFMPEGVIDEOENCODER_H

extern "C"
{
#include <ffmpeg/libavcodec/avcodec.h>
#include <ffmpeg/libavformat/avformat.h>
#include <ffmpeg/libavutil/opt.h>
#include <ffmpeg/libavutil/imgutils.h>
}

#include "Codec.h"

class FfmpegVideoEncoder : public VideoEncoder {
    const char* TAG = "FfmpegVideoEncoder";
    //默认编码比特率
    const int DEFAULT_BIT_RATE = 400000;

public:
    FfmpegVideoEncoder();

    void init(MediaInfo *header) override;

    int send_frame(MediaData *inFrame) override;

    int receive_packet(MediaData *outPacket) override;

    void release() override;
public:
    bool isInitSuccess;
    MediaInfo mAVHeader{};
    AVCodec *mCodec;
    AVCodecContext *mCodecContext;
    AVFrame *mInFrame;
    AVPacket *mOutPacket;
    int mPts;
};


#endif //GPLAYER_FFMPEGVIDEOENCODER_H
