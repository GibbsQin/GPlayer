#include "MediaDataJni.h"
#include "../base/Log.h"
#include "../utils/JniHelper.h"

#define TAG "MediaDataJni"

jclass MediaDataJni::avdataClass = nullptr;
jmethodID MediaDataJni::avdataConstructor = nullptr;
jfieldID MediaDataJni::dataFieldId = nullptr;
jfieldID MediaDataJni::sizeFieldId = nullptr;
jfieldID MediaDataJni::data1FieldId = nullptr;
jfieldID MediaDataJni::size1FieldId = nullptr;
jfieldID MediaDataJni::data2FieldId = nullptr;
jfieldID MediaDataJni::size2FieldId = nullptr;
jfieldID MediaDataJni::ptsFieldId = nullptr;
jfieldID MediaDataJni::dtsFieldId = nullptr;
jfieldID MediaDataJni::widthFieldId = nullptr;
jfieldID MediaDataJni::heightFieldId = nullptr;
jfieldID MediaDataJni::flagFieldId = nullptr;

/**
 * 初始化jni相关数据
 */
void MediaDataJni::initClassAndMethodJni() {
    if (avdataClass != nullptr) {
        return;
    }

    JNIEnv *env = JniHelper::getJNIEnv();
    if (env == nullptr) {
        LOGD(TAG, "env is nullptr");
        return;
    }

    avdataClass = env->FindClass("com/gibbs/gplayer/media/MediaData");
    avdataClass = (jclass) (env->NewGlobalRef(avdataClass));
    avdataConstructor = env->GetMethodID(avdataClass, "<init>", "()V");
    dataFieldId = env->GetFieldID(avdataClass, "data", "Ljava/nio/ByteBuffer;");
    sizeFieldId = env->GetFieldID(avdataClass, "size", "I");
    data1FieldId = env->GetFieldID(avdataClass, "data1", "Ljava/nio/ByteBuffer;");
    size1FieldId = env->GetFieldID(avdataClass, "size1", "I");
    data2FieldId = env->GetFieldID(avdataClass, "data2", "Ljava/nio/ByteBuffer;");
    size2FieldId = env->GetFieldID(avdataClass, "size2", "I");
    ptsFieldId = env->GetFieldID(avdataClass, "pts", "J");
    dtsFieldId = env->GetFieldID(avdataClass, "dts", "J");
    widthFieldId = env->GetFieldID(avdataClass, "width", "I");
    heightFieldId = env->GetFieldID(avdataClass, "height", "I");
    flagFieldId = env->GetFieldID(avdataClass, "flag", "I");
}

jobject MediaDataJni::createJObject() {
    JNIEnv *env = JniHelper::getJNIEnv();
    if (env == nullptr) {
        LOGD(TAG, "obj or env is nullptr");
        return nullptr;
    }
    jobject avdataObj = env->NewObject(avdataClass, avdataConstructor);

    return avdataObj;
}

jobject MediaDataJni::createJObject(MediaData *mediaData) {
    JNIEnv *env = JniHelper::getJNIEnv();
    if (env == nullptr) {
        LOGD(TAG, "obj or env is nullptr");
        return nullptr;
    }
    jobject avdataObj = env->NewObject(avdataClass, avdataConstructor);
    if (mediaData->data != nullptr && mediaData->size > 0) {
        jobject byteBuffer = JniHelper::createByteBuffer(env, mediaData->data, mediaData->size);
        env->SetObjectField(avdataObj, dataFieldId, byteBuffer);
        env->SetIntField(avdataObj, sizeFieldId, mediaData->size);
        env->DeleteLocalRef(byteBuffer);
    }
    if (mediaData->data1 != nullptr && mediaData->size1 > 0) {
        jobject byteBuffer = JniHelper::createByteBuffer(env, mediaData->data1, mediaData->size1);
        env->SetObjectField(avdataObj, data1FieldId, byteBuffer);
        env->SetIntField(avdataObj, size1FieldId, mediaData->size1);
        env->DeleteLocalRef(byteBuffer);
    }
    if (mediaData->data2 != nullptr && mediaData->size2 > 0) {
        jobject byteBuffer = JniHelper::createByteBuffer(env, mediaData->data2, mediaData->size2);
        env->SetObjectField(avdataObj, data2FieldId, byteBuffer);
        env->SetIntField(avdataObj, size2FieldId, mediaData->size2);
        env->DeleteLocalRef(byteBuffer);
    }
    env->SetLongField(avdataObj, ptsFieldId, mediaData->pts);
    env->SetLongField(avdataObj, dtsFieldId, mediaData->dts);
    env->SetIntField(avdataObj, widthFieldId, mediaData->width);
    env->SetIntField(avdataObj, heightFieldId, mediaData->height);
    env->SetIntField(avdataObj, flagFieldId, mediaData->flag);

    return avdataObj;
}

jobject
MediaDataJni::createJObject(MediaData *avdata, jobject byteBuffer, jobject byteBuffer1, jobject byteBuffer2) {
    JNIEnv *env = JniHelper::getJNIEnv();
    if (env == nullptr) {
        LOGD(TAG, "obj or env is nullptr");
        return nullptr;
    }
    jobject avdataObj = env->NewObject(avdataClass, avdataConstructor);
    if (avdata->data != nullptr && avdata->size > 0 && byteBuffer != nullptr) {
        auto *pData = (jbyte *) env->GetDirectBufferAddress(byteBuffer);
        memcpy(pData, avdata->data, avdata->size);
        env->SetObjectField(avdataObj, dataFieldId, byteBuffer);
        env->SetIntField(avdataObj, sizeFieldId, avdata->size);
    }
    if (avdata->data1 != nullptr && avdata->size1 > 0 && byteBuffer1 != nullptr) {
        auto *pData = (jbyte *) env->GetDirectBufferAddress(byteBuffer1);
        memcpy(pData, avdata->data1, avdata->size1);
        env->SetObjectField(avdataObj, data1FieldId, byteBuffer1);
        env->SetIntField(avdataObj, size1FieldId, avdata->size1);
    }
    if (avdata->data2 != nullptr && avdata->size2 > 0 && byteBuffer2 != nullptr) {
        auto *pData = (jbyte *) env->GetDirectBufferAddress(byteBuffer2);
        memcpy(pData, avdata->data2, avdata->size2);
        env->SetObjectField(avdataObj, data2FieldId, byteBuffer2);
        env->SetIntField(avdataObj, size2FieldId, avdata->size2);
    }
    env->SetLongField(avdataObj, ptsFieldId, avdata->pts);
    env->SetLongField(avdataObj, dtsFieldId, avdata->dts);
    env->SetIntField(avdataObj, widthFieldId, avdata->width);
    env->SetIntField(avdataObj, heightFieldId, avdata->height);
    env->SetIntField(avdataObj, flagFieldId, avdata->flag);

    return avdataObj;
}

void MediaDataJni::copyToAVData(jobject jobj, MediaData *avdata) {
    JNIEnv *env = JniHelper::getJNIEnv();
    if (env == nullptr || jobj == nullptr) {
        LOGE(TAG, "obj or env is nullptr");
        return;
    }
    avdata->pts = static_cast<uint64_t>(env->GetLongField(jobj, ptsFieldId));
    avdata->dts = static_cast<uint64_t>(env->GetLongField(jobj, dtsFieldId));
    avdata->width = static_cast<uint32_t>(env->GetIntField(jobj, widthFieldId));
    avdata->height = static_cast<uint32_t>(env->GetIntField(jobj, heightFieldId));
    avdata->flag = static_cast<uint8_t>(env->GetIntField(jobj, flagFieldId));
    jobject byteBuffer = env->GetObjectField(jobj, dataFieldId);
    int size = env->GetIntField(jobj, sizeFieldId);
    if (byteBuffer != nullptr && size > 0) {
        auto *pData = (jbyte *) env->GetDirectBufferAddress(byteBuffer); //获取buffer数据首地址
        if (pData != nullptr) {
            memcpy(avdata->data, (uint8_t *) pData, static_cast<size_t>(size));
            avdata->size = static_cast<uint32_t>(size);
        }
    }

    byteBuffer = env->GetObjectField(jobj, data1FieldId);
    size = env->GetIntField(jobj, size1FieldId);
    if (byteBuffer != nullptr && size > 0) {
        auto *pData = (jbyte *) env->GetDirectBufferAddress(byteBuffer); //获取buffer数据首地址
        if (pData != nullptr) {
            memcpy(avdata->data1, (uint8_t *) pData, static_cast<size_t>(size));
            avdata->size1 = static_cast<uint32_t>(size);
        }
    }

    byteBuffer = env->GetObjectField(jobj, data2FieldId);
    size = env->GetIntField(jobj, size2FieldId);
    if (byteBuffer != nullptr && size > 0) {
        auto *pData = (jbyte *) env->GetDirectBufferAddress(byteBuffer); //获取buffer数据首地址
        if (pData != nullptr) {
            memcpy(avdata->data2, (uint8_t *) pData, static_cast<size_t>(size));
            avdata->size2 = static_cast<uint32_t>(size);
        }
    }
}

/**
 * java对象转换为AVData(浅拷贝)
 * @param
 * @return
 */
void MediaDataJni::shallowCopyToAVData(jobject jobj, MediaData *avdata) {
    JNIEnv *env = JniHelper::getJNIEnv();
    if (env == nullptr) {
        LOGD(TAG, "obj or env is nullptr");
        return;
    }
    avdata->pts = static_cast<uint64_t>(env->GetLongField(jobj, ptsFieldId));
    avdata->dts = static_cast<uint64_t>(env->GetLongField(jobj, dtsFieldId));
    avdata->width = static_cast<uint32_t>(env->GetIntField(jobj, widthFieldId));
    avdata->height = static_cast<uint32_t>(env->GetIntField(jobj, heightFieldId));
    avdata->flag = static_cast<uint8_t>(env->GetIntField(jobj, flagFieldId));
    jobject byteBuffer = env->GetObjectField(jobj, dataFieldId);
    int size = env->GetIntField(jobj, sizeFieldId);
    //jlong dwCapacity = env->GetDirectBufferCapacity(byteBuffer);      //获取buffer的容量
    if (byteBuffer != nullptr && size > 0) {
        auto *pData = (jbyte *) env->GetDirectBufferAddress(byteBuffer); //获取buffer数据首地址
        if (pData != nullptr) {
            avdata->data = (uint8_t *) pData;
            avdata->size = static_cast<uint32_t>(size);
        }
    }

    byteBuffer = env->GetObjectField(jobj, data1FieldId);
    size = env->GetIntField(jobj, size1FieldId);
    if (byteBuffer != nullptr && size > 0) {
        auto *pData = (jbyte *) env->GetDirectBufferAddress(byteBuffer); //获取buffer数据首地址
        if (pData != nullptr) {
            avdata->data1 = (uint8_t *) pData;
            avdata->size1 = static_cast<uint32_t>(size);
        }
    }

    byteBuffer = env->GetObjectField(jobj, data2FieldId);
    size = env->GetIntField(jobj, size2FieldId);
    if (byteBuffer != nullptr && size > 0) {
        auto *pData = (jbyte *) env->GetDirectBufferAddress(byteBuffer); //获取buffer数据首地址
        if (pData != nullptr) {
            avdata->data2 = (uint8_t *) pData;
            avdata->size2 = static_cast<uint32_t>(size);
        }
    }
}
