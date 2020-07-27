//
// Created by Gibbs on 2020/7/18.
//

#ifndef GPLAYER_INTERCEPTOR_H
#define GPLAYER_INTERCEPTOR_H

#include "media/Media.h"

#define AV_TYPE_AUDIO 0
#define AV_TYPE_VIDEO 1

class Interceptor {
public:
    virtual ~Interceptor(){};

    virtual int onInit(MediaInfo* header) = 0;

    virtual int inputBuffer(MediaData *buffer, int type) = 0;

    virtual int outputBuffer(MediaData **buffer, int type) = 0;

    virtual void onRelease() = 0;
};

#endif //GPLAYER_INTERCEPTOR_H
