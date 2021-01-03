/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#ifndef GPLAYER_YUVVIDEORENDERER_H
#define GPLAYER_YUVVIDEORENDERER_H


#include "EglRenderer.h"
#include "source/FrameSource.h"
#include "VideoRenderer.h"

class YuvVideoRenderer : public VideoRenderer {
public:
    YuvVideoRenderer(FrameSource *source);

    ~YuvVideoRenderer();

    void surfaceCreated(ANativeWindow *window, int videoWidth, int videoHeight);

    void surfaceChanged(int width, int height);

    void surfaceDestroyed();

    uint64_t render(uint64_t nowMs);

    bool isRenderValid() {
        return isEglInit;
    }

private:
    EglRenderer *eglRenderer;
    YuvGlesProgram *glProgram;
    FrameSource *mediaSource;
    bool isEglInit = false;
};


#endif //GPLAYER_YUVVIDEORENDERER_H
