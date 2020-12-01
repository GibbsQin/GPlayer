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
Java_com_gibbs_gplayer_GPlayer_nInit(JNIEnv *env, jobject clazz, jint channelId, jobject jAVSource) {
    auto pGPlayerImp = new GPlayerEngine(channelId, jAVSource);
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