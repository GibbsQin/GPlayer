/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#ifndef GPLAYER_GPLAYERJNI_H
#define GPLAYER_GPLAYERJNI_H

#include <jni.h>
#include <string>

extern "C" {
#include <demuxing/avformat_def.h>
}

class GPlayerJni {
public:
    GPlayerJni(jobject obj);

    ~GPlayerJni();

    void onMessageCallback(int msgId, int arg1, long arg2, const char *msg1, const char *msg2);

    void onMessageCallback(int msgId, int arg1, long arg2, const char *msg1, const char *msg2, jobject obj);

private:
    jobject playerJObj;
    jmethodID onMessageCallbackMethod;
};


#endif //GPLAYER_GPLAYERJNI_H
