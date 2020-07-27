#ifndef GPLAYER_MEDIA_INFO_JNI_H
#define GPLAYER_MEDIA_INFO_JNI_H

#include <jni.h>

extern "C" {
#include <ffmpeg/libavcodec/avcodec.h>
};

#include "Codec.h"

class MediaInfoJni {
public:
    /**
     * 初始化jni相关数据
     */
    static void initClassAndMethodJni();

    /**
     * 创建java对象
     * @param avdata
     * @return
     */
    static jobject createJobject(MediaInfo *avdata);

    /**
     * java对象转换为AVHeader
     * @param
     * @return
     */
    static void copyToAVHeader(jobject jobj, MediaInfo *mediaInfo);

    static jclass mediaInfoClass;
    static jmethodID mediaInfoConstructor;
    static jmethodID setIntegerMethodId;
    static jmethodID getIntegerMethodId;
};

#endif //GPLAYER_MEDIA_INFO_JNI_H
