//
// Created by Gibbs on 2020/12/20.
//

#include <base/LoopThread.h>
#include "RenderHelper.h"

RenderHelper::RenderHelper(FrameSource *source, MessageQueue *messageQueue) {
    audioRenderer = new AudioRenderer(source);
    videoRenderer = new VideoRenderer(source);
    this->messageQueue = messageQueue;
}

RenderHelper::~RenderHelper() {
    delete audioRenderer;
    delete videoRenderer;
}

void RenderHelper::setSurface(ANativeWindow *window) {
    nativeWindow = window;
}

void RenderHelper::setAudioTrack(AudioTrackJni *track) {
    audioRenderer->setAudioTrack(track);
}

void RenderHelper::setVideoParams(int width, int height) {
    videoWidth = width;
    videoHeight = height;
}

void RenderHelper::setAudioParams(int sampleRate, int channels, int format, int bytesPerSample) {
    audioRenderer->setAudioParams(sampleRate, channels, format, bytesPerSample);
}

void RenderHelper::initAudioRenderer() {
    audioRenderer->init();
}

void RenderHelper::initVideoRenderer() {
    videoRenderer->surfaceCreated(nativeWindow, videoWidth, videoHeight);
}

int RenderHelper::renderAudio(int arg1, long arg2) {
    uint64_t temp = audioRenderer->render(nowPts);
    if (temp > 0) {
        if (nowPts == 0) {
            messageQueue->pushMessage(MSG_DOMAIN_STATE, STATE_PLAYING, 0);
        }
        nowPts = temp;
    }
    return 0;
}

int RenderHelper::renderVideo(int arg1, long arg2) {
    if (!videoRenderer->isRenderValid()) {
        return ERROR_EXIST;
    }
    videoRenderer->render(nowPts);
    return 0;
}

void RenderHelper::releaseAudioRenderer() {
    audioRenderer->release();
}

void RenderHelper::releaseVideoRenderer() {
    videoRenderer->surfaceDestroyed();
}
