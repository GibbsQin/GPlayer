/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#ifndef GPLAYER_RENDERHELPER_H
#define GPLAYER_RENDERHELPER_H


#include <render/AudioRenderer.h>
#include <render/YuvVideoRenderer.h>
#include <source/MessageSource.h>

#define FELLOW_AUDIO 0
#define FELLOW_CLOCK 1

#define MAX_PTS 9223372036854775807

class RenderHelper {
public:
    RenderHelper(FrameSource *source, MessageSource *messageSource, int mediaCodecFlag,
                 bool hasAudio, bool hasVideo);

    ~RenderHelper();

    void setNativeWindow(ANativeWindow *window);

    void setAudioTrack(AudioTrackJni *track);

    void setVideoParams(int width, int height);

    void setAudioParams(int sampleRate, int channels, int format, int bytesPerSample);

    void initAudioRenderer();

    void initVideoRenderer();

    int renderAudio(int arg1, long arg2);

    int renderVideo(int arg1, long arg2);

    void releaseAudioRenderer();

    void releaseVideoRenderer();

    void setStopWhenEmpty(bool enable) {
        stopWhenEmpty = enable;
    }

private:
    AudioRenderer *audioRenderer = nullptr;
    VideoRenderer *videoRenderer = nullptr;
    MessageSource *messageSource = nullptr;
    ANativeWindow *nativeWindow = nullptr;
    int videoWidth = 0;
    int videoHeight = 0;
    uint64_t nowPts = 0;
    uint64_t startPts = 0;
    uint64_t firstFramePts = 0;
    bool hasNotifyFirstFrame = false;
    bool stopWhenEmpty = false;
    bool hasAudio = false;
    bool hasVideo = false;
    int avSyncMethod = FELLOW_AUDIO;
};


#endif //GPLAYER_RENDERHELPER_H
