//
// Created by Gibbs on 2020/7/16.
//

#ifndef GPLAYER_GPLAYERIMP_H
#define GPLAYER_GPLAYERIMP_H

#include <interceptor/CodecInterceptor.h>
#include "CommonThread.h"
#include "MediaSource.h"
#include "MediaSourceJni.h"

#define ENGINE_FREQUENCY 80
#define MAX_OUTPUT_FRAME_SIZE 5
#define SLEEP_TIME_GAP 2000

class GPlayerImp {

public:
    GPlayerImp(jobject jAVSource);

    ~GPlayerImp();

    void onInit();

    void onRelease();

    MediaSource *getInputSource() const;

    MediaSourceJni *getOutputSource() const;

public:

    void snapShot(const std::string &filePath);

    void startRecordVideo(const std::string &filePath);

    void stopRecordVideo();

    void onVideoRecordResult(int code, const string &path);

private:

    void onAudioThreadStart();

    void processAudioBuffer(int64_t time);

    void onAudioThreadEnd();

    void onVideoThreadStart();

    void processVideoBuffer(int64_t time);

    void onVideoThreadEnd();

    void onAllThreadEnd();

private:
    MediaSource *inputSource;
    MediaSourceJni *outputSource;

private:
    bool decodeFlag;
    bool mediaCodecFlag;
    CommonThread *audioEngineThread;
    CommonThread *videoEngineThread;
    bool isAudioThreadRunning{};
    bool isVideoThreadRunning{};
    bool hasInit{};
    bool isRelease{};
    bool isStopping{};
    int mRemoteAudioQueueSize;
    int mRemoteVideoQueueSize;
    long mAudioSleepTimeUs;
    long mVideoSleepTimeUs;
    Interceptor *codeInterceptor;
    std::mutex releaseMutex;
};


#endif //GPLAYER_GPLAYERIMP_H
