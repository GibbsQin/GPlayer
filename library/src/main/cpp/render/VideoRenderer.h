/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#ifndef GPLAYER_VIDEORENDERER_H
#define GPLAYER_VIDEORENDERER_H


class VideoRenderer {
public:
    virtual ~VideoRenderer() {};

    virtual void surfaceCreated(ANativeWindow *window, int videoWidth, int videoHeight) = 0;

    virtual void surfaceChanged(int width, int height) = 0;

    virtual void surfaceDestroyed() = 0;

    virtual uint64_t render(uint64_t nowMs) = 0;

    virtual bool isRenderValid() = 0;
};


#endif //GPLAYER_VIDEORENDERER_H
