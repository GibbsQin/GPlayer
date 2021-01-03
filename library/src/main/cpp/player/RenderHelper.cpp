/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#include <base/LoopThread.h>
#include <render/SurfaceVideoRenderer.h>
#include "RenderHelper.h"

RenderHelper::RenderHelper(FrameSource *source, MessageSource *messageSource, int mediaCodecFlag) {
    audioRenderer = new AudioRenderer(source);
    if (mediaCodecFlag) {
        videoRenderer = new SurfaceVideoRenderer(source);
    } else {
        videoRenderer = new YuvVideoRenderer(source);
    }
    this->messageSource = messageSource;
}

RenderHelper::~RenderHelper() {
    delete audioRenderer;
    delete videoRenderer;
}

void RenderHelper::setNativeWindow(ANativeWindow *window) {
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
    hasNotifyFirstFrame = false;
}

int RenderHelper::renderAudio(int arg1, long arg2) {
    uint64_t temp = audioRenderer->render(nowPts);
    if (temp > 0) {
        if (!hasNotifyFirstFrame) {
            hasNotifyFirstFrame = true;
            messageSource->pushMessage(MSG_DOMAIN_STATE, STATE_PLAYING, 0);
        }
        nowPts = temp;
        int nowPtsSecond = nowPts / 1000 / 1000;
        messageSource->pushMessage(MSG_DOMAIN_TIME, nowPtsSecond, 0);
    } else {
        if (stopWhenEmpty) {
            stopWhenEmpty = false;
            return ERROR_EXIST;
        }
    }
    return 0;
}

int RenderHelper::renderVideo(int arg1, long arg2) {
    if (!videoRenderer->isRenderValid()) {
        return ERROR_EXIST;
    }
    if (!videoRenderer->render(nowPts)) {
        if (stopWhenEmpty) {
            stopWhenEmpty = false;
            return ERROR_EXIST;
        }
    }
    return 0;
}

void RenderHelper::releaseAudioRenderer() {
    audioRenderer->release();
}

void RenderHelper::releaseVideoRenderer() {
    videoRenderer->surfaceDestroyed();
}
