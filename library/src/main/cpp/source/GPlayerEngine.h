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
    GPlayerEngine(jobject jAVSource);

    ~GPlayerEngine();

    void onInit();

    void onRelease();

    MediaSource *getInputSource() const;

    MediaSourceJni *getOutputSource() const;

    bool isDemuxingLoop() { return isDemuxing; }

    void stopDemuxingLoop();

private:
    void startDemuxing(char *web_url, int channelId, FfmpegCallback callback, MediaInfo *mediaInfo);

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
    bool decodeFlag;
    bool mediaCodecFlag;
    CommonThread *audioEngineThread;
    CommonThread *videoEngineThread;
    DemuxingThread *demuxingThread;
    bool isRelease{};
    bool isStopping{};
    Interceptor *codeInterceptor;
    char *mUrl;
    bool isDemuxing;
};


#endif //GPLAYER_GPLAYERENGINE_H
