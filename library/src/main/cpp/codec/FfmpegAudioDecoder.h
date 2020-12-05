#ifndef GPLAYER_FFMPEGAUDIODECODER_H
#define GPLAYER_FFMPEGAUDIODECODER_H
extern "C"
{
#include <ffmpeg/libavcodec/avcodec.h>
#include <ffmpeg/libavformat/avformat.h>
#include <ffmpeg/libswresample/swresample.h>
}

#include "Codec.h"

//#define SAVE_DECODE_FILE 1
#define ENABLE_PARSER 1

#define TAG "FfmpegAudioDecoder"

class FfmpegAudioDecoder : public AudioDecoder {
public:
    FfmpegAudioDecoder();

    ~FfmpegAudioDecoder();

    void init(MediaInfo *header) override;

    virtual int send_packet(MediaData *inPacket) override;

    virtual int receive_frame(MediaData *outFrame) override;

    void release() override;
private:

#ifdef SAVE_DECODE_FILE
    FILE *audioFile;
#endif
    bool isInitSuccess;
    MediaInfo *mHeader;

    AVCodec *mCodec;
    AVCodecContext *mCodecContext;
#ifdef ENABLE_PARSER
    AVCodecParserContext *mParser;
#endif
    AVPacket *mInPacket;
    AVFrame *mOutFrame;
};


#endif //GPLAYER_FFMPEGAUDIODECODER_H
