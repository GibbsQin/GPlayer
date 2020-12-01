//
// Created by Gibbs on 2020/7/16.
//

#ifndef GPLAYER_GPLAYERENGINE_H
#define GPLAYER_GPLAYERENGINE_H

#include <interceptor/CodecInterceptor.h>
#include "CommonThread.h"
#include "MediaSource.h"
#include "MediaSourceJni.h"
#include "DemuxingThread.h"

extern "C" {
#include <protocol/remuxing.h>
}

class GPlayerEngine {

public:
    GPlayerEngine(int channelId, jobject jAVSource);

    ~GPlayerEngine();

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

    bool isDemuxingLoop() { return isDemuxing; }

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
    MediaSourceJni *outputSource;

private:
    bool mediaCodecFlag;
    CommonThread *audioEngineThread;
    CommonThread *videoEngineThread;
    DemuxingThread *demuxingThread;
    Interceptor *codeInterceptor;
    char *mUrl;
    int mChannelId;
    bool isDemuxing;
};


#endif //GPLAYER_GPLAYERENGINE_H
