/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#include <base/Log.h>
#include <j4a/JniHelper.h>
#include "GPlayerJni.h"

#define TAG "MediaSourceJni"

GPlayerJni::GPlayerJni(jobject obj) {
    LOGI(TAG, "create GPlayerJni");
    JNIEnv *env = JniHelper::getJNIEnv();
    playerJObj = env->NewGlobalRef(obj);
    jclass sourceClass = env->GetObjectClass(obj);
    onMessageCallbackMethod = env->GetMethodID(sourceClass, "onMessageCallback",
                                               "(IIJLjava/lang/String;Ljava/lang/String;Ljava/lang/Object;)V");
}

GPlayerJni::~GPlayerJni() {
    bool attach = JniHelper::attachCurrentThread();
    JNIEnv *env = JniHelper::getJNIEnv();
    if (playerJObj != nullptr && env) {
        env->DeleteGlobalRef(playerJObj);
        playerJObj = nullptr;
        onMessageCallbackMethod = nullptr;
    }
    if (attach) {
        JniHelper::detachCurrentThread();
    }
    LOGE(TAG, "GPlayerJni destroyed");
}

void
GPlayerJni::onMessageCallback(int msgId, int arg1, long arg2, const char *msg1, const char *msg2) {
    onMessageCallback(msgId, arg1, arg2, msg1, msg2, (jobject)nullptr);
}

void
GPlayerJni::onMessageCallback(int msgId, int arg1, long arg2, const char *msg1, const char *msg2, jobject obj) {
    if (playerJObj != nullptr && onMessageCallbackMethod != nullptr) {
        bool attach = JniHelper::attachCurrentThread();

        JNIEnv *env = JniHelper::getJNIEnv();
        jstring jMsg1 = msg1 ? JniHelper::newStringUTF(env, msg1) : nullptr;
        jstring jMsg2 = msg2 ? JniHelper::newStringUTF(env, msg2) : nullptr;
        JniHelper::callVoidMethod(playerJObj, onMessageCallbackMethod, msgId, arg1, arg2, jMsg1,
                                  jMsg2, obj);
        if (jMsg1) {
            env->DeleteLocalRef(jMsg1);
        }
        if (jMsg2) {
            env->DeleteLocalRef(jMsg2);
        }
        if (attach) {
            JniHelper::detachCurrentThread();
        }
    }
}
