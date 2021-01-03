/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#include <thread>
#include "YuvVideoRenderer.h"
#include "media/MediaData.h"

YuvVideoRenderer::YuvVideoRenderer(FrameSource *source) {
    LOGI("VideoRenderer", "CoreFlow : new YuvVideoRenderer");
    eglRenderer = new EglRenderer();
    glProgram = new YuvGlesProgram();
    mediaSource = source;
}

YuvVideoRenderer::~YuvVideoRenderer() {
    delete eglRenderer;
    eglRenderer = nullptr;
    delete glProgram;
    glProgram = nullptr;
}

void YuvVideoRenderer::surfaceCreated(ANativeWindow *window, int videoWidth, int videoHeight) {
    eglRenderer->setWindow(window);
    eglRenderer->setVideoSize(videoWidth, videoHeight);
    if (eglRenderer->initialize()) {
        isEglInit = glProgram->buildProgram();
    }

    LOGE("VideoRenderer", "surfaceCreated isEglInit %d", isEglInit);
}

void YuvVideoRenderer::surfaceChanged(int width, int height) {

}

void YuvVideoRenderer::surfaceDestroyed() {
    eglRenderer->destroy();
}

uint64_t YuvVideoRenderer::render(uint64_t nowMs) {
    MediaData *mediaData;
    if (mediaSource->readVidFrame(&mediaData) > 0) {
        if (nowMs > mediaData->pts) {
            glProgram->buildTextures(mediaData->data, mediaData->data1, mediaData->data2,
                                       mediaData->width, mediaData->height);
            glProgram->drawFrame();
            eglRenderer->swapBuffers();
            mediaSource->popVidFrame(mediaData);
            return mediaData->pts;
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    return 0;
}
