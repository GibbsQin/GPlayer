//
// Created by Gibbs on 2020/7/16.
//

#include <media/MediaInfoJni.h>
#include <media/MediaDataJni.h>
#include <base/Log.h>
#include <utils/JniHelper.h>
#include "MediaSourceJni.h"

#define TAG "MediaSourceJni"

MediaSourceJni::MediaSourceJni(jobject obj) {
    LOGI(TAG, "create MediaSourceJni");
    createJni(obj);
}

MediaSourceJni::~MediaSourceJni() {
    destroyJni();
    LOGE(TAG, "MediaSourceJni destroyed");
}

void MediaSourceJni::createJni(jobject obj) {
    JNIEnv *env = JniHelper::getJNIEnv();
    if (env == nullptr) {
        LOGE(TAG, "env is nullptr");
        return;
    }

    p2pSourceJObj = env->NewGlobalRef(obj);
    jclass p2pSourceClass = env->GetObjectClass(obj);
    getUrlMethodId = env->GetMethodID(p2pSourceClass, "getUrl", "()Ljava/lang/String;");
    getFlagMethodId = env->GetMethodID(p2pSourceClass, "getFlag", "()I");
    onInitMethodId = env->GetMethodID(p2pSourceClass, "onInit",
                                      "(ILcom/gibbs/gplayer/media/MediaInfo;)V");
    onReceiveAudioMethodId = env->GetMethodID(p2pSourceClass, "onReceiveAudio",
                                              "(Lcom/gibbs/gplayer/media/MediaData;)I");
    onReceiveVideoMethodId = env->GetMethodID(p2pSourceClass, "onReceiveVideo",
                                              "(Lcom/gibbs/gplayer/media/MediaData;)I");
    onReleaseMethodId = env->GetMethodID(p2pSourceClass, "onRelease", "()V");
    onErrorMethodId = env->GetMethodID(p2pSourceClass, "onError", "(ILjava/lang/String;)V");
    onAudioPacketSizeChangedMethodId = env->GetMethodID(p2pSourceClass, "onRemoteAudioSizeChanged",
                                                        "(I)V");
    onVideoPacketSizeChangedMethodId = env->GetMethodID(p2pSourceClass, "onRemoteVideoSizeChanged",
                                                        "(I)V");
}

void MediaSourceJni::destroyJni() {
    bool attach = JniHelper::attachCurrentThread();
    JNIEnv *env = JniHelper::getJNIEnv();
    if (p2pSourceJObj && env) {
        env->DeleteGlobalRef(p2pSourceJObj);
        p2pSourceJObj = nullptr;
        getUrlMethodId = nullptr;
        getFlagMethodId = nullptr;
        onInitMethodId = nullptr;
        onReceiveAudioMethodId = nullptr;
        onReceiveVideoMethodId = nullptr;
        onReleaseMethodId = nullptr;
        onErrorMethodId = nullptr;
        onAudioPacketSizeChangedMethodId = nullptr;
        onVideoPacketSizeChangedMethodId = nullptr;
    }
    if (attach) {
        JniHelper::detachCurrentThread();
    }
}

int MediaSourceJni::sendAudio2Java(MediaData *outFrame) {
    int size = -1;
    if (p2pSourceJObj && onReceiveAudioMethodId && outFrame) {
        bool attach = JniHelper::attachCurrentThread();
        jobject jdata;
        jdata = MediaDataJni::createJObject(outFrame);
        size = JniHelper::callIntMethod(p2pSourceJObj, onReceiveAudioMethodId, jdata);
        JniHelper::deleteLocalRef(jdata);
        if (attach) {
            JniHelper::detachCurrentThread();
        }
    }
    return size;
}

int MediaSourceJni::sendVideo2Java(MediaData *outFrame) {
    int size = -1;
    if (p2pSourceJObj && onReceiveVideoMethodId && outFrame) {
        bool attach = JniHelper::attachCurrentThread();
        jobject jdata;
        jdata = MediaDataJni::createJObject(outFrame);
        size = JniHelper::callIntMethod(p2pSourceJObj, onReceiveVideoMethodId, jdata);
        JniHelper::deleteLocalRef(jdata);
        if (attach) {
            JniHelper::detachCurrentThread();
        }
    }
    return size;
}

void MediaSourceJni::sendAudioPacketSize2Java(int size) {
    if (p2pSourceJObj && onAudioPacketSizeChangedMethodId) {
        bool attach = JniHelper::attachCurrentThread();

        JniHelper::callVoidMethod(p2pSourceJObj, onAudioPacketSizeChangedMethodId, size);
        if (attach) {
            JniHelper::detachCurrentThread();
        }
    }
}

void MediaSourceJni::sendVideoPacketSize2Java(int size) {
    if (p2pSourceJObj && onVideoPacketSizeChangedMethodId) {
        bool attach = JniHelper::attachCurrentThread();

        JniHelper::callVoidMethod(p2pSourceJObj, onVideoPacketSizeChangedMethodId, size);
        if (attach) {
            JniHelper::detachCurrentThread();
        }
    }
}

void MediaSourceJni::callJavaInitMethod(MediaInfo *header, int channelId) {
    LOGI(TAG, "callJavaInitMethod");
    if (p2pSourceJObj && onInitMethodId) {
        bool attach = JniHelper::attachCurrentThread();
        jobject jdata = MediaInfoJni::createJobject(header);
        JniHelper::callVoidMethod(p2pSourceJObj, onInitMethodId, channelId, jdata);
        JniHelper::deleteLocalRef(jdata);
        if (attach) {
            JniHelper::detachCurrentThread();
        }
    }
}

void MediaSourceJni::callJavaReleaseMethod() {
    LOGI(TAG, "callJavaReleaseMethod");
    if (p2pSourceJObj && onReleaseMethodId) {
        bool attach = JniHelper::attachCurrentThread();

        JniHelper::callVoidMethod(p2pSourceJObj, onReleaseMethodId);
        if (attach) {
            JniHelper::detachCurrentThread();
        }
    }
}

void MediaSourceJni::callJavaErrorMethod(int errorCode, const string &errorMessage) {
    if (p2pSourceJObj && onErrorMethodId) {
        bool attach = JniHelper::attachCurrentThread();

        JNIEnv *env = JniHelper::getJNIEnv();
        jstring jErrorMsg = JniHelper::newStringUTF(env, errorMessage.c_str());
        JniHelper::callVoidMethod(p2pSourceJObj, onErrorMethodId, errorCode, jErrorMsg);
        env->DeleteLocalRef(jErrorMsg);
        if (attach) {
            JniHelper::detachCurrentThread();
        }
    }
}

string MediaSourceJni::getUrl() {
    string url;
    if (p2pSourceJObj && getUrlMethodId) {
        jobject jobj = JniHelper::callObjectMethod(p2pSourceJObj, getUrlMethodId);
        if (jobj != nullptr) {
            auto urlStr = (jstring) (jobj);
            JNIEnv *env = JniHelper::getJNIEnv();
            url = JniHelper::getStringUTF(env, urlStr);
        }
    }
    return url;
}

int MediaSourceJni::getFlag() {
    int flag = 0;
    if (p2pSourceJObj && getFlagMethodId) {
        flag = JniHelper::callIntMethod(p2pSourceJObj, getFlagMethodId);
    }
    return flag;
}

