//
// Created by Gibbs on 2020/7/16.
//

#ifndef GPLAYER_GPLAYER_H
#define GPLAYER_GPLAYER_H

#include <interceptor/CodecInterceptor.h>
#include <player/GPlayerJni.h>
#include "DecodeThread.h"
#include "DemuxingThread.h"
#include "MediaSource.h"
extern "C" {
#include <demuxing/demuxing.h>
}

#define AV_FLAG_SOURCE_MEDIA_CODEC 0x00000002

#define MAX_BUFFER_SIZE 30

class GPlayer {

public:
    GPlayer(int channelId, int flag, std::string url, jobject obj);

    ~GPlayer();

public:
    void av_init(MediaInfo *header);

    uint32_t av_feed_audio(uint8_t *pInputBuf, uint32_t dwInputDataSize,
                           uint64_t u64InputPTS, uint64_t u64InputDTS, int flag);

    uint32_t av_feed_video(uint8_t *pInputBuf, uint32_t dwInputDataSize,
                           uint64_t u64InputPTS, uint64_t u64InputDTS, int flag);

    void av_destroy();

    void av_error(int code, char *msg);

public:
    void start();

    void stop();

    void startDemuxing(char *web_url, int channelId, FfmpegCallback callback, MediaInfo *mediaInfo);

    LoopFlag isDemuxingLoop();

    MediaSource* getFrameSource();

private:
    void startDecode();

    void stopDecode();

    void onAudioThreadStart();

    int processAudioBuffer();

    void onAudioThreadEnd();

    void onVideoThreadStart();

    int processVideoBuffer();

    void onVideoThreadEnd();

private:
    MediaSource *inputSource;
    MediaSource *outputSource;
    GPlayerJni *playerJni;

private:
    bool mediaCodecFlag;
    char *mUrl;
    int mChannelId;
    bool isDemuxing{};
    DecodeThread *audioEngineThread;
    DecodeThread *videoEngineThread;
    DemuxingThread *demuxingThread{};
    Interceptor *codeInterceptor;
};


#endif //GPLAYER_GPLAYER_H
