/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#ifndef GPLAYER_SURFACEVIDEORENDERER_H
#define GPLAYER_SURFACEVIDEORENDERER_H


#include <android/native_window.h>
#include "VideoRenderer.h"
#include "source/FrameSource.h"

class SurfaceVideoRenderer : public VideoRenderer {
public:
    SurfaceVideoRenderer(FrameSource *source);

    ~SurfaceVideoRenderer();

    void surfaceCreated(ANativeWindow *window, int videoWidth, int videoHeight);

    void surfaceChanged(int width, int height);

    void surfaceDestroyed();

    uint64_t render(uint64_t nowMs);

    bool isRenderValid() {
        return true;
    }

private:
    FrameSource *mediaSource;
};


#endif //GPLAYER_SURFACEVIDEORENDERER_H
