//
// Created by qinshenghua on 2020/12/14.
//

#include "VideoRenderer.h"
#include "../media/MediaData.h"

VideoRenderer::VideoRenderer(FrameSource *source) {
    eglRenderer = new EglRenderer();
    mediaSource = source;
}

VideoRenderer::~VideoRenderer() {
    delete eglRenderer;
    eglRenderer = nullptr;
}

void VideoRenderer::surfaceCreated(ANativeWindow *window, int videoWidth, int videoHeight) {
    eglRenderer->setWindow(window);
    eglRenderer->setVideoSize(videoWidth, videoHeight);
    isEglInit = eglRenderer->initialize();
}

void VideoRenderer::surfaceChanged(int width, int height) {

}

void VideoRenderer::surfaceDestroyed() {
    eglRenderer->destroy();
}

uint64_t VideoRenderer::render(uint64_t nowMs) {
    MediaData *mediaData;
    if (mediaSource->readVideoBuffer(&mediaData) > 0) {
        LOGI("VideoRenderer", "render video %lld", mediaData->pts);
        if (nowMs > mediaData->pts) {
            eglRenderer->buildTextures(mediaData->data, mediaData->data1, mediaData->data2,
                                       mediaData->width, mediaData->height);
            eglRenderer->drawFrame();
            mediaSource->popVideoBuffer();
            return mediaData->pts;
        }
    }
    return -1;
}
