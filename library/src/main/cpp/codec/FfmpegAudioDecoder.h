#ifndef GPLAYER_FFMPEGAUDIODECODER_H
#define GPLAYER_FFMPEGAUDIODECODER_H
extern "C"
{
#include <ffmpeg/libavcodec/avcodec.h>
#include <ffmpeg/libavformat/avformat.h>
#include <ffmpeg/libswresample/swresample.h>
}

#include "Codec.h"

#define TAG "FfmpegAudioDecoder"

class FfmpegAudioDecoder : public AudioDecoder {
public:
    FfmpegAudioDecoder();

    ~FfmpegAudioDecoder();

    void init(AVCodecParameters *codecParameters) override;

    virtual int send_packet(AVPacket *inPacket) override;

    virtual int receive_frame(MediaData *outFrame) override;

    void release() override;
private:

    bool isInitSuccess;
    AVCodecContext *mCodecContext;
    AVFrame *mOutFrame;
};


#endif //GPLAYER_FFMPEGAUDIODECODER_H
