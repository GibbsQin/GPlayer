#include <jni.h>
#include <string>
#include <source/MediaSource.h>
#include <utils/JniHelper.h>
#include <base/Log.h>
#include <source/MediaPipe.h>
#include <media/MediaInfoJni.h>
#include <media/MediaDataJni.h>

extern "C" {
#include <protocol/protocol.h>
#include <protocol/avformat_def.h>
#include <codec/ffmpeg/libavcodec/jni.h>
#include <protocol/remuxing.h>
}

//#define ENABLE_FFMPEG_JNI 1

static FfmpegCallback sFfmpegCallback;

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGI("GPlayerC", "JNI_OnLoad");
    JNIEnv *env = nullptr;
    jint result = JNI_ERR;
    JniHelper::sJavaVM = vm;

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("GPlayerC", "JNI_OnLoad fail %d", result);
        return result;
    }

    sFfmpegCallback.av_format_init = &(MediaPipe::av_format_init);
    sFfmpegCallback.av_format_extradata_audio = &(MediaPipe::av_format_extradata_audio);
    sFfmpegCallback.av_format_extradata_video = &(MediaPipe::av_format_extradata_video);
    sFfmpegCallback.av_format_feed_audio = &(MediaPipe::av_format_feed_audio);
    sFfmpegCallback.av_format_feed_video = &(MediaPipe::av_format_feed_video);
    sFfmpegCallback.av_format_destroy = &(MediaPipe::av_format_destroy);
    sFfmpegCallback.av_format_error = &(MediaPipe::av_format_error);
    sFfmpegCallback.av_format_loop_wait = &(MediaPipe::av_format_loop_wait);

    MediaInfoJni::initClassAndMethodJni();
    MediaDataJni::initClassAndMethodJni();

#ifdef ENABLE_FFMPEG_JNI
    int init_ffmpeg_jni_result = av_jni_set_java_vm(vm, nullptr);
    if (init_ffmpeg_jni_result < 0) {
        LOGE("GPlayerC", "av_jni_set_java_vm fail : %s", av_err2str(init_ffmpeg_jni_result));
    }
#endif

    result = JNI_VERSION_1_4;

    return result;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_gibbs_gplayer_GPlayer_nInitAVSource(JNIEnv *env, jobject clazz, jobject jAVSource) {
    auto pGPlayerImp = new GPlayerEngine(jAVSource);
    std::string url = pGPlayerImp->getOutputSource()->getUrl();
    LOGI("GPlayerC", "Java_com_gibbs_gplayer_GPlayer_nInitAVSource url = %s", url.c_str());
    int channelId = 1;
    auto *mediaInfo = new MediaInfo();
    if (!url.empty()) {
        if (url.find("file:") == 0) {
            string fileUrl = url.substr(5, url.length());
            LOGI("GPlayerC", "nInitFileSource channelId = %d, url = %s", channelId, fileUrl.c_str());
            pGPlayerImp->getInputSource()->setChannelId(channelId);
            MediaPipe::sGPlayerMap[channelId] = pGPlayerImp;
            start_demuxing(PROTOCOL_TYPE_FILE, fileUrl.c_str(), sFfmpegCallback, mediaInfo, &channelId);
        } else {
            LOGI("GPlayerC", "nInitStreamSource channelId = %d, url = %s", channelId, url.c_str());
            pGPlayerImp->getInputSource()->setChannelId(channelId);
            MediaPipe::sGPlayerMap[channelId] = pGPlayerImp;
            start_demuxing(PROTOCOL_TYPE_STREAM, url.c_str(), sFfmpegCallback, mediaInfo, &channelId);
        }
    }
    return channelId;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nFinish(JNIEnv *env, jobject thiz, jlong channel_id,
                                       jboolean force) {
    LOGI("GPlayerC", "nFinish channelId : %lld, force : %d", channel_id, force);
    auto targetPlayer = MediaPipe::sGPlayerMap[channel_id];
    if (!targetPlayer) {
        return;
    }
    auto targetSource = targetPlayer->getInputSource();
    auto outputSource = targetPlayer->getOutputSource();
    if (!targetSource || !outputSource) {
        return;
    }
    string url = outputSource->getUrl();
    int type = -1;
    if (!url.empty()) {
        if (url.find("file:") == 0) {
            type = PROTOCOL_TYPE_FILE;
        } else {
            type = PROTOCOL_TYPE_STREAM;
        }
    }
    targetSource->pendingFlushBuffer();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_gibbs_gplayer_GPlayer_nDestroyAVSource(JNIEnv *env, jobject clazz, jlong channel_id) {
    LOGI("GPlayerC", "nDestroyAVSource channelId : %lld", channel_id);
    MediaPipe::deleteFromMap(channel_id);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_gibbs_gplayer_codec_C_getMimeByCodecType(JNIEnv *env, jclass clazz, jint type) {
    char* mime = getMimeByCodeID((CODEC_TYPE) type);
    return JniHelper::newStringUTF(env, mime);
}