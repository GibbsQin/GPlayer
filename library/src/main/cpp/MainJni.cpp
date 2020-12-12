#include <jni.h>
#include <utils/JniHelper.h>
#include <base/Log.h>
#include <source/MediaPipe.h>
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
                                     jobject player) {
    auto pGPlayerImp = new GPlayer(channelId, flag, player);
    MediaPipe::sGPlayerMap[channelId] = pGPlayerImp;
    LOGI("GPlayerJni", "CoreFlow : nInit player count %d", MediaPipe::sGPlayerMap.size());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nPrepare(JNIEnv *env, jobject thiz, jint channel_id, jstring url) {
    LOGI("GPlayerJni", "nPrepare channelId : %d", channel_id);
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return;
    }
    targetPlayer->prepare(JniHelper::getStringUTF(env, url));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nStart(JNIEnv *env, jobject thiz, jint channel_id) {
    LOGI("GPlayerJni", "nStart channelId : %d", channel_id);
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return;
    }
    targetPlayer->start();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nPause(JNIEnv *env, jobject thiz, jint channel_id) {
    LOGI("GPlayerJni", "nPause channelId : %d", channel_id);
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return;
    }
    targetPlayer->pause();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nResume(JNIEnv *env, jobject thiz, jint channel_id) {
    LOGI("GPlayerJni", "nResume channelId : %d", channel_id);
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return;
    }
    targetPlayer->resume();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nSeekTo(JNIEnv *env, jobject thiz, jint channel_id, jint second_ms) {
    LOGI("GPlayerJni", "nSeekTo channelId : %d, %d", channel_id, second_ms);
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return;
    }
    targetPlayer->seekTo(second_ms);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nStop(JNIEnv *env, jobject thiz, jint channel_id,
                                     jboolean force) {
    LOGI("GPlayerJni", "nStop channelId : %d, force : %d", channel_id, force);
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return;
    }
    targetPlayer->stop();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nRelease(JNIEnv *env, jobject clazz, jint channel_id) {
    LOGI("GPlayerJni", "nDestroyAVSource channelId : %d", channel_id);
    MediaPipe::deleteFromMap(static_cast<int>(channel_id));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nSetFlags(JNIEnv *env, jobject thiz, jint channel_id, jint flags) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return;
    }
    targetPlayer->setFlags(flags);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_readAudioSource(JNIEnv *env, jobject thiz,
                                                             jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return nullptr;
    }
    MediaData *mediaData = nullptr;
    targetPlayer->getOutputSource()->readAudioBuffer(&mediaData);
    if (mediaData == nullptr) {
        return nullptr;
    }
    return MediaDataJni::createJObject(mediaData);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_readVideoSource(JNIEnv *env, jobject thiz,
                                                             jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return nullptr;
    }
    MediaData *mediaData = nullptr;
    targetPlayer->getOutputSource()->readVideoBuffer(&mediaData);
    if (mediaData == nullptr) {
        return nullptr;
    }
    return MediaDataJni::createJObject(mediaData);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_removeFirstAudioPackage(JNIEnv *env, jobject thiz,
                                                                     jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return;
    }
    targetPlayer->getOutputSource()->popAudioBuffer();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_removeFirstVideoPackage(JNIEnv *env, jobject thiz,
                                                                     jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return;
    }
    targetPlayer->getOutputSource()->popVideoBuffer();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_flushBuffer(JNIEnv *env, jobject thiz,
                                                         jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return;
    }
    targetPlayer->getOutputSource()->flushBuffer();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_getAudioBufferSize(JNIEnv *env, jobject thiz,
                                                                jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return -1;
    }
    return targetPlayer->getOutputSource()->getAudioBufferSize();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_getVideoBufferSize(JNIEnv *env, jobject thiz,
                                                                jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return -1;
    }
    return targetPlayer->getOutputSource()->getVideoBufferSize();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_getFrameRate(JNIEnv *env, jobject thiz,
                                                          jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return -1;
    }
    return targetPlayer->getInputSource()->getFormatInfo()->vidframerate;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_getDuration(JNIEnv *env, jobject thiz,
                                                         jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return -1;
    }
    return targetPlayer->getInputSource()->getFormatInfo()->duration;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_getSampleRate(JNIEnv *env, jobject thiz,
                                                           jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return -1;
    }
    return targetPlayer->getInputSource()->getAudioAVCodecParameters()->sample_rate;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_getSampleFormat(JNIEnv *env, jobject thiz,
                                                             jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return -1;
    }
    return targetPlayer->getInputSource()->getAudioAVCodecParameters()->format;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_getChannels(JNIEnv *env, jobject thiz,
                                                         jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return -1;
    }
    return targetPlayer->getInputSource()->getAudioAVCodecParameters()->channels;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_getWidth(JNIEnv *env, jobject thiz, jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return -1;
    }
    return targetPlayer->getInputSource()->getVideoAVCodecParameters()->width;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_getHeight(JNIEnv *env, jobject thiz, jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return -1;
    }
    return targetPlayer->getInputSource()->getVideoAVCodecParameters()->height;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_getRotate(JNIEnv *env, jobject thiz, jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return -1;
    }

    return targetPlayer->getInputSource()->getFormatInfo()->vidrotate;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_gibbs_gplayer_source_MediaSourceImp_getBytesPerFrame(JNIEnv *env, jobject thiz,
                                                              jint channel_id) {
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return -1;
    }

    return targetPlayer->getInputSource()->getAudioAVCodecParameters()->frame_size;
}