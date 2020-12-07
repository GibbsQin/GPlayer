#ifndef GPLAYER_MEDIA_DATA_JNI_H
#define GPLAYER_MEDIA_DATA_JNI_H

#include <stdlib.h>
#include <jni.h>
#include "Codec.h"

class MediaDataJni {
public:
    /**
     * 初始化jni相关数据
     */
    static void initClassAndMethodJni();

    /**
     * 创建java对象
     * @return
     */
    static jobject createJObject();

    /**
     * 创建java对象
     * @param mediaData
     * @return
     */
    static jobject createJObject(MediaData *mediaData);

    /**
     * 创建java对象
     * @return
     */
    static jobject createJObject(MediaData *avdata, jobject byteBuffer, jobject byteBuffer1, jobject byteBuffer2);

    /**
     * java对象转换为AVData
     * @param
     * @return
     */
    static void copyToAVData(jobject jobj, MediaData *avdata);

    /**
     * java对象转换为AVData(浅拷贝)
     * @param
     * @return
     */
    static void shallowCopyToAVData(jobject jobj, MediaData *avdata);

private:
    static jclass avdataClass;
    static jmethodID avdataConstructor;
    static jfieldID dataFieldId;
    static jfieldID sizeFieldId;
    static jfieldID data1FieldId;
    static jfieldID size1FieldId;
    static jfieldID data2FieldId;
    static jfieldID size2FieldId;
    static jfieldID ptsFieldId;
    static jfieldID dtsFieldId;
    static jfieldID widthFieldId;
    static jfieldID heightFieldId;
    static jfieldID flagFieldId;
};

#endif //GPLAYER_MEDIA_DATA_JNI_H
