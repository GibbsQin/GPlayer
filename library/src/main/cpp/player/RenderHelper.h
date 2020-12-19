//
// Created by Gibbs on 2020/12/20.
//

#ifndef GPLAYER_RENDERHELPER_H
#define GPLAYER_RENDERHELPER_H


#include <render/AudioRenderer.h>
#include <render/VideoRenderer.h>
#include <source/MessageQueue.h>

class RenderHelper {
public:
    RenderHelper(FrameSource *source, MessageQueue *messageQueue);

    ~RenderHelper();

    void setSurface(ANativeWindow *window);

    void setAudioTrack(AudioTrackJni *track);

    void setVideoParams(int width, int height);

    void setAudioParams(int sampleRate, int channels, int format, int bytesPerSample);

    void initAudioRenderer();

    void initVideoRenderer();

    int renderAudio(int arg1, long arg2);

    int renderVideo(int arg1, long arg2);

    void releaseAudioRenderer();

    void releaseVideoRenderer();

private:
    AudioRenderer *audioRenderer;
    VideoRenderer *videoRenderer;
    MessageQueue *messageQueue;
    ANativeWindow *nativeWindow = nullptr;
    int videoWidth = 0;
    int videoHeight = 0;
    uint64_t nowPts = 0;
};


#endif //GPLAYER_RENDERHELPER_H
