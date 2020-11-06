#include "MediaInfoJni.h"
#include "../utils/JniHelper.h"
#include "../base/Log.h"

#define TAG "MediaInfoJni"

/**
 * 文件长度(毫秒)
 * The associated value is an integer
 */
#define KEY_DURATION  "duration"

/**
 * 视频编码类型 h264 h265
 * The associated value is an integer
 */
#define KEY_VIDEO_TYPE  "video-type"

/**
 * A key describing the width of the content in a video format.
 * The associated value is an integer
 */
#define KEY_WIDTH "width"

/**
 * A key describing the height of the content in a video format.
 * The associated value is an integer
 */
#define KEY_HEIGHT "height"

/**
 * frame rate
 * The associated value is an integer
 */
#define KEY_FRAME_RATE "frame-rate"

/**
 * A key describing the average bitrate in bits/sec.
 * The associated value is an integer
 */
#define KEY_BIT_RATE "bitrate"


//音频编码格式
#define KEY_AUDIO_TYPE  "audio-type"

//音频编码的参数
#define KEY_AUDIO_CODEC_OPTION "audio-codec-option"

// Codec-specific bitstream restrictions that the stream conforms to.
#define KEY_AUDIO_PROFILE "audio-profile"

// 音频模式： 单声道/双声道
#define KEY_AUDIO_MODE "audio-mode"

// 声道数
#define KEY_AUDIO_CHANNELS "audio-channels"

// 音频位宽，目前只支持16bit
#define KEY_AUDIO_BIT_WIDTH "audio-bit-width"

// 音频采样率
#define KEY_AUDIO_SAMPLE_RATE "audio-sample-rate"

// 每帧数据里的采样数
#define KEY_AUDIO_SAMPLE_NUM_PERFRAME "audio-sample-num-perframe"

// 视频旋转角度
#define KEY_VIDEO_ROTATE "video-rotate"

jclass MediaInfoJni::mediaInfoClass = nullptr;
jmethodID MediaInfoJni::mediaInfoConstructor = nullptr;
jmethodID MediaInfoJni::setIntegerMethodId = nullptr;
jmethodID MediaInfoJni::getIntegerMethodId = nullptr;

void MediaInfoJni::initClassAndMethodJni() {
    if (mediaInfoClass != nullptr) {
        return;
    }

    JNIEnv *env = JniHelper::getJNIEnv();
    if (env == nullptr) {
        LOGD(TAG, "env is nullptr");
        return;
    }

    mediaInfoClass = env->FindClass("com/gibbs/gplayer/media/MediaInfo");
    if (mediaInfoClass) {
        mediaInfoClass = (jclass) (env->NewGlobalRef(mediaInfoClass));
    }
    mediaInfoConstructor = env->GetMethodID(mediaInfoClass, "<init>", "()V");
    setIntegerMethodId = env->GetMethodID(mediaInfoClass, "setInteger", "(Ljava/lang/String;I)V");
    getIntegerMethodId = env->GetMethodID(mediaInfoClass, "getInteger", "(Ljava/lang/String;I)I");
}

jobject MediaInfoJni::createJobject(MediaInfo *avdata) {
    JNIEnv *env = JniHelper::getJNIEnv();
    if (env == nullptr) {
        LOGD(TAG, "env is nullptr");
        return nullptr;
    }
    jobject avheaderObj = env->NewObject(mediaInfoClass, mediaInfoConstructor);

    jstring key = JniHelper::newStringUTF(env, KEY_DURATION);
    JniHelper::callVoidMethod(avheaderObj, setIntegerMethodId, key, avdata->duration);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_VIDEO_TYPE);
    JniHelper::callVoidMethod(avheaderObj, setIntegerMethodId, key, avdata->videoType);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_WIDTH);
    JniHelper::callVoidMethod(avheaderObj, setIntegerMethodId, key, avdata->videoWidth);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_HEIGHT);
    JniHelper::callVoidMethod(avheaderObj, setIntegerMethodId, key, avdata->videoHeight);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_FRAME_RATE);
    JniHelper::callVoidMethod(avheaderObj, setIntegerMethodId, key, avdata->videoFrameRate);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_AUDIO_TYPE);
    JniHelper::callVoidMethod(avheaderObj, setIntegerMethodId, key, avdata->audioType);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_AUDIO_CODEC_OPTION);
    JniHelper::callVoidMethod(avheaderObj, setIntegerMethodId, key, avdata->audioCodecOption);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_AUDIO_PROFILE);
    JniHelper::callVoidMethod(avheaderObj, setIntegerMethodId, key, avdata->audioProfile);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_AUDIO_MODE);
    JniHelper::callVoidMethod(avheaderObj, setIntegerMethodId, key, avdata->audioMode);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_AUDIO_CHANNELS);
    JniHelper::callVoidMethod(avheaderObj, setIntegerMethodId, key, avdata->audioChannels);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_AUDIO_BIT_WIDTH);
    JniHelper::callVoidMethod(avheaderObj, setIntegerMethodId, key, avdata->audioBitWidth);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_AUDIO_SAMPLE_RATE);
    JniHelper::callVoidMethod(avheaderObj, setIntegerMethodId, key, avdata->audioSampleRate);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_AUDIO_SAMPLE_NUM_PERFRAME);
    JniHelper::callVoidMethod(avheaderObj, setIntegerMethodId, key, avdata->sampleNumPerFrame);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_VIDEO_ROTATE);
    JniHelper::callVoidMethod(avheaderObj, setIntegerMethodId, key, avdata->videoRotate);
    env->DeleteLocalRef(key);

    return avheaderObj;
}

void MediaInfoJni::copyToAVHeader(jobject jobj, MediaInfo *mediaInfo) {
    if (jobj == nullptr || mediaInfo == nullptr) {
        LOGD(TAG, "param is nullptr");
        return;
    }

    JNIEnv *env = JniHelper::getJNIEnv();
    if (env == nullptr) {
        LOGD(TAG, "env is nullptr");
        return;
    }
    jstring key = JniHelper::newStringUTF(env, KEY_VIDEO_TYPE);
    mediaInfo->videoType = JniHelper::callIntMethod(jobj, getIntegerMethodId, key, 0);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_WIDTH);
    mediaInfo->videoWidth = JniHelper::callIntMethod(jobj, getIntegerMethodId, key, 0);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_HEIGHT);
    mediaInfo->videoHeight = JniHelper::callIntMethod(jobj, getIntegerMethodId, key, 0);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_FRAME_RATE);
    mediaInfo->videoFrameRate = JniHelper::callIntMethod(jobj, getIntegerMethodId, key, 0);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_AUDIO_TYPE);
    mediaInfo->audioType = JniHelper::callIntMethod(jobj, getIntegerMethodId, key, 0);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_AUDIO_MODE);
    mediaInfo->audioMode = JniHelper::callIntMethod(jobj, getIntegerMethodId, key, 0);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_AUDIO_BIT_WIDTH);
    mediaInfo->audioBitWidth = JniHelper::callIntMethod(jobj, getIntegerMethodId, key, 0);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_AUDIO_SAMPLE_RATE);
    mediaInfo->audioSampleRate = JniHelper::callIntMethod(jobj, getIntegerMethodId, key, 0);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_AUDIO_SAMPLE_NUM_PERFRAME);
    mediaInfo->sampleNumPerFrame = JniHelper::callIntMethod(jobj, getIntegerMethodId, key, 0);
    env->DeleteLocalRef(key);

    key = JniHelper::newStringUTF(env, KEY_VIDEO_ROTATE);
    mediaInfo->videoRotate = JniHelper::callIntMethod(jobj, getIntegerMethodId, key, 0);
    env->DeleteLocalRef(key);
}
