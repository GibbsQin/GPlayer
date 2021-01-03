//
// Created by qinshenghua on 2020/12/14.
//

#ifndef GPLAYER_VIDEORENDERER_H
#define GPLAYER_VIDEORENDERER_H


#include "EglRenderer.h"
#include "source/FrameSource.h"

class VideoRenderer {
public:
    VideoRenderer(FrameSource *source);

    ~VideoRenderer();

    void surfaceCreated(ANativeWindow *window, int videoWidth, int videoHeight);

    void surfaceChanged(int width, int height);

    void surfaceDestroyed();

    uint64_t render(uint64_t nowMs);

    bool isRenderValid() {
        return isEglInit;
    }

private:
    EglRenderer *eglRenderer;
    FrameSource *mediaSource;
    bool isEglInit = false;
};


#endif //GPLAYER_VIDEORENDERER_H
