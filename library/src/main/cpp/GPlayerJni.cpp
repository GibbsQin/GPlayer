#include <jni.h>
#include <utils/JniHelper.h>
#include <base/Log.h>
#include <source/MediaPipe.h>
#include <media/MediaInfoJni.h>
#include <media/MediaDataJni.h>

//#define ENABLE_FFMPEG_JNI 1

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGI("GPlayerJni", "JNI_OnLoad");
    JNIEnv *env = nullptr;
    jint result = JNI_ERR;
    JniHelper::sJavaVM = vm;

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("GPlayerJni", "JNI_OnLoad fail %d", result);
        return result;
    }

    MediaPipe::initFfmpegCallback();
    MediaInfoJni::initClassAndMethodJni();
    MediaDataJni::initClassAndMethodJni();

#ifdef ENABLE_FFMPEG_JNI
    int init_ffmpeg_jni_result = av_jni_set_java_vm(vm, nullptr);
    if (init_ffmpeg_jni_result < 0) {
        LOGE("GPlayerJni", "av_jni_set_java_vm fail : %s", av_err2str(init_ffmpeg_jni_result));
    }
#endif

    result = JNI_VERSION_1_4;

    return result;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nInit(JNIEnv *env, jobject clazz, jint channelId, jint flag,
                                     jstring url, jobject player) {
    auto pGPlayerImp = new GPlayer(channelId, flag, JniHelper::getStringUTF(env, url), player);
    MediaPipe::sGPlayerMap[channelId] = pGPlayerImp;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nStart(JNIEnv *env, jobject thiz, jlong channel_id) {
    LOGI("GPlayerJni", "nStart channelId : %lld", channel_id);
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return;
    }
    targetPlayer->start();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nStop(JNIEnv *env, jobject thiz, jlong channel_id,
                                     jboolean force) {
    LOGI("GPlayerJni", "nStop channelId : %lld, force : %d", channel_id, force);
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return;
    }
    targetPlayer->stop();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nDestroy(JNIEnv *env, jobject clazz, jlong channel_id) {
    LOGI("GPlayerJni", "nDestroyAVSource channelId : %lld", channel_id);
    MediaPipe::deleteFromMap(static_cast<int>(channel_id));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_nGetAVHeader(JNIEnv *env, jobject thiz,
                                                          jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return nullptr;
    }
    MediaInfo *header = targetPlayer->getFrameSource()->getAVHeader();
    return MediaInfoJni::createJobject(header);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_nReadAudioSource(JNIEnv *env, jobject thiz,
                                                              jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return nullptr;
    }
    MediaData *mediaData;
    targetPlayer->getFrameSource()->readAudioBuffer(&mediaData);
    return MediaDataJni::createJObject(mediaData);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_nReadVideoSource(JNIEnv *env, jobject thiz,
                                                              jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return nullptr;
    }
    MediaData *mediaData;
    targetPlayer->getFrameSource()->readVideoBuffer(&mediaData);
    return MediaDataJni::createJObject(mediaData);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_nRemoveFirstAudioPackage(JNIEnv *env, jobject thiz,
                                                                      jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return;
    }
    targetPlayer->getFrameSource()->popAudioBuffer();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_nRemoveFirstVideoPackage(JNIEnv *env, jobject thiz,
                                                                      jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return;
    }
    targetPlayer->getFrameSource()->popVideoBuffer();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_nFlushBuffer(JNIEnv *env, jobject thiz,
                                                          jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return;
    }
    targetPlayer->getFrameSource()->flushBuffer();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_nGetAudioBufferSize(JNIEnv *env, jobject thiz,
                                                                 jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return -1;
    }
    return targetPlayer->getFrameSource()->getAudioBufferSize();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_nGetVideoBufferSize(JNIEnv *env, jobject thiz,
                                                                 jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return -1;
    }
    return targetPlayer->getFrameSource()->getVideoBufferSize();
}