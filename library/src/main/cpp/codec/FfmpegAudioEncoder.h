#ifndef GPLAYER_FFMPEGAUDIOENCODER_H
#define GPLAYER_FFMPEGAUDIOENCODER_H
extern "C"
{
#include <ffmpeg/libavcodec/avcodec.h>
#include <ffmpeg/libavformat/avformat.h>
#include <ffmpeg/libswresample/swresample.h>
}

#include <codec/ffmpeg/libavcodec/avcodec.h>
#include "Codec.h"

typedef struct {
    int write_adts;
    int objecttype;
    int sample_rate_index;
    int channel_conf;
} ADTSContext;

#define ADD_ADTS_TO_AVPACKET 1

#define ADTS_HEADER_SIZE 7

class FfmpegAudioEncoder : public AudioEncoder{
    const char* TAG = "FfmpegAudioEncoder";

public:
    FfmpegAudioEncoder();

    void init(MediaInfo* header) override;

    int send_frame(MediaData *inFrame) override;

    int receive_packet(MediaData *outPacket) override;

    void release() override;

    int aac_decode_extradata(ADTSContext *adts, unsigned char *pbuf, int bufsize);

    int aac_set_adts_head(ADTSContext *acfg, unsigned char *buf, int size);

public:
    bool isInitSuccess;
    MediaInfo mHeader{};

    AVCodec *mCodec;
    AVCodecContext *mCodecContext;
    AVFrame *mOrgAvFrame;
    AVFrame *mSwrFrame;
    AVPacket *mOutPacket;
    SwrContext *mSwrCtx;
    AVSampleFormat mOriginAVSampleFormat;
    long mPts;
    bool mIsNeedSwr{};
#ifdef ADD_ADTS_TO_AVPACKET
    unsigned char mADTSHeader[ADTS_HEADER_SIZE] = {0};
    ADTSContext mADTSContext{};
#endif
};


#endif //GPLAYER_FFMPEGAUDIOENCODER_H
