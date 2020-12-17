//
// Created by qinshenghua on 2020/12/14.
//

#ifndef GPLAYER_VIDEORENDERER_H
#define GPLAYER_VIDEORENDERER_H


#include "YuvGlesProgram.h"
#include "source/FrameSource.h"

class VideoRenderer {
public:
    VideoRenderer(FrameSource *source);

    ~VideoRenderer();

    void init();

    void render(uint64_t nowMs);

    void release();

private:
    FrameSource *mediaSource;
    YuvGlesProgram *glesProgram;
};


#endif //GPLAYER_VIDEORENDERER_H
