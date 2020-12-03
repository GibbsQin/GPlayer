//
// Created by Gibbs on 2020/7/16.
//

#ifndef GPLAYER_GPLAYERJNI_H
#define GPLAYER_GPLAYERJNI_H

using namespace std;

#include <jni.h>
#include <string>

class GPlayerJni {
public:
    GPlayerJni(jobject obj);

    ~GPlayerJni();

    void onMessageCallback(int msgId, int arg1, int arg2, char* msg1, char* msg2);

private:
    jobject playerJObj{};
    jmethodID onMessageCallbackMethod{};
};


#endif //GPLAYER_GPLAYERJNI_H
