#include <jni.h>
#include <j4a/JniHelper.h>
#include <base/Log.h>
#include <player/GPlayer.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

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
JNIEXPORT jlong JNICALL
Java_com_gibbs_gplayer_GPlayer_nInit(JNIEnv *env, jobject clazz, jint flag, jobject player) {
    auto pGPlayerImp = new GPlayer(flag, player);
    return reinterpret_cast<jlong>(pGPlayerImp);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nSetSurface(JNIEnv *env, jobject thiz, jlong nativePlayer,
                                           jobject surface) {
    LOGI("GPlayerJni", "nSetSurface channelId : %d", nativePlayer);
    auto targetPlayer = reinterpret_cast<GPlayer *>(nativePlayer);
    if (!targetPlayer) {
        return;
    }
    if (surface) {
        ANativeWindow *win = ANativeWindow_fromSurface(env, surface);
        targetPlayer->setSurface(win);
    } else {
        targetPlayer->setSurface(nullptr);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nSetAudioTrack(JNIEnv *env, jobject thiz, jlong nativePlayer,
                                              jobject audio_track_wrap) {
    LOGI("GPlayerJni", "nSetAudioTrack channelId : %d", nativePlayer);
    auto targetPlayer = reinterpret_cast<GPlayer *>(nativePlayer);
    if (!targetPlayer) {
        return;
    }
    targetPlayer->setAudioTrack(new AudioTrackJni(audio_track_wrap));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nPrepare(JNIEnv *env, jobject thiz, jlong nativePlayer, jstring url) {
    LOGI("GPlayerJni", "nPrepare channelId : %d", nativePlayer);
    auto targetPlayer = reinterpret_cast<GPlayer *>(nativePlayer);
    if (!targetPlayer) {
        return;
    }
    targetPlayer->prepare(JniHelper::getStringUTF(env, url));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nStart(JNIEnv *env, jobject thiz, jlong nativePlayer) {
    LOGI("GPlayerJni", "nStart channelId : %d", nativePlayer);
    auto *targetPlayer = reinterpret_cast<GPlayer *>(nativePlayer);
    if (!targetPlayer) {
        return;
    }
    targetPlayer->start();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nPause(JNIEnv *env, jobject thiz, jlong nativePlayer) {
    LOGI("GPlayerJni", "nPause channelId : %d", nativePlayer);
    auto *targetPlayer = reinterpret_cast<GPlayer *>(nativePlayer);
    if (!targetPlayer) {
        return;
    }
    targetPlayer->pause();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nResume(JNIEnv *env, jobject thiz, jlong nativePlayer) {
    LOGI("GPlayerJni", "nResume channelId : %d", nativePlayer);
    auto *targetPlayer = reinterpret_cast<GPlayer *>(nativePlayer);
    if (!targetPlayer) {
        return;
    }
    targetPlayer->resume();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nSeekTo(JNIEnv *env, jobject thiz, jlong nativePlayer, jint second_ms) {
    LOGI("GPlayerJni", "nSeekTo channelId : %d, %d", nativePlayer, second_ms);
    auto *targetPlayer = reinterpret_cast<GPlayer *>(nativePlayer);
    if (!targetPlayer) {
        return;
    }
    targetPlayer->seekTo(static_cast<uint32_t>(second_ms));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nStop(JNIEnv *env, jobject thiz, jlong nativePlayer,
                                     jboolean force) {
    LOGI("GPlayerJni", "nStop channelId : %d, force : %d", nativePlayer, force);
    auto *targetPlayer = reinterpret_cast<GPlayer *>(nativePlayer);
    if (!targetPlayer) {
        return;
    }
    targetPlayer->stop();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nRelease(JNIEnv *env, jobject clazz, jlong nativePlayer) {
    LOGI("GPlayerJni", "nDestroyAVSource channelId : %d", nativePlayer);
    auto *targetPlayer = reinterpret_cast<GPlayer *>(nativePlayer);
    if (!targetPlayer) {
        return;
    }
    LOGI("GPlayerJni", "delete GPlayer");
    delete targetPlayer;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nSetFlags(JNIEnv *env, jobject thiz, jlong nativePlayer, jint flags) {
    auto *targetPlayer = reinterpret_cast<GPlayer *>(nativePlayer);
    if (!targetPlayer) {
        return;
    }
    targetPlayer->setFlags(static_cast<uint32_t>(flags));
}
