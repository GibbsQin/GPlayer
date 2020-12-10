#include <media/MediaDataJni.h>
#include <base/Log.h>
#include "OutputSource.h"

#define TAG "OutputSource"

OutputSource::OutputSource() {
    LOGI(TAG, "CoreFlow : create OutputSource");
    mFormatInfo = static_cast<FormatInfo *>(malloc(sizeof(FormatInfo)));
    mFormatInfo->audcodecpar = avcodec_parameters_alloc();
    mFormatInfo->vidcodecpar = avcodec_parameters_alloc();
    mFormatInfo->subcodecpar = avcodec_parameters_alloc();
}

OutputSource::~OutputSource() {
    LOGI(TAG, "CoreFlow : OutputSource destroyed %d %d",
         audioPacketQueue.size(), videoPacketQueue.size());
    avcodec_parameters_free(&mFormatInfo->audcodecpar);
    avcodec_parameters_free(&mFormatInfo->vidcodecpar);
    avcodec_parameters_free(&mFormatInfo->subcodecpar);
    free(mFormatInfo);
}

void OutputSource::onInit(FormatInfo *formatInfo) {
    mFormatInfo = formatInfo;
}

uint32_t OutputSource::onReceiveAudio(MediaData *inPacket) {
    uint32_t queueSize = 0;
    audioPacketQueue.push_back(inPacket);
    queueSize = static_cast<uint32_t>(audioPacketQueue.size());
    LOGI(TAG, "queue audio packet %lld", inPacket->pts);
    return queueSize;
}

uint32_t OutputSource::onReceiveVideo(MediaData *inPacket) {
    uint32_t queueSize = 0;
    videoPacketQueue.push_back(inPacket);
    queueSize = static_cast<uint32_t>(videoPacketQueue.size());
    LOGI(TAG, "queue video packet %lld", inPacket->pts);
    return queueSize;
}

void OutputSource::onRelease() {
}

FormatInfo *OutputSource::getFormatInfo() {
    return mFormatInfo;
}

AVCodecParameters *OutputSource::getAudioAVCodecParameters() {
    return mFormatInfo->audcodecpar;
}

AVCodecParameters *OutputSource::getVideoAVCodecParameters() {
    return mFormatInfo->vidcodecpar;
}

AVCodecParameters *OutputSource::getSubtitleAVCodecParameters() {
    return mFormatInfo->subcodecpar;
}

int OutputSource::readAudioBuffer(MediaData **avData) {
    if (audioPacketQueue.size() <= 0) {
        return AV_SOURCE_EMPTY;
    }
    *avData = audioPacketQueue.front();
    if (!(*avData)) {
        LOGE(TAG, "readAudioBuffer item is null");
        popAudioBuffer();
        return 0;
    }
    return static_cast<int>(audioPacketQueue.size());
}

int OutputSource::readVideoBuffer(MediaData **avData) {
    if (videoPacketQueue.size() <= 0) {
        return AV_SOURCE_EMPTY;
    }
    *avData = videoPacketQueue.front();
    if (!(*avData)) {
        LOGE(TAG, "readVideoBuffer item is null");
        popVideoBuffer();
        return 0;
    }
    return static_cast<int>(videoPacketQueue.size());
}

void OutputSource::popAudioBuffer() {
    mAudioLock.lock();
    if (audioPacketQueue.size() > 0) {
        audioPacketQueue.pop_front();
        LOGI(TAG, "deque audio packet");
    }
    mAudioLock.unlock();
}

void OutputSource::popVideoBuffer() {
    mVideoLock.lock();
    if (videoPacketQueue.size() > 0) {
        videoPacketQueue.pop_front();
        LOGI(TAG, "deque video packet");
    }
    mVideoLock.unlock();
}

void OutputSource::flushBuffer() {
    LOGI(TAG, "CoreFlow : flushBuffer start");
    mVideoLock.lock();
    mAudioLock.lock();
    flushAudioBuffer();
    flushVideoBuffer();
    mAudioLock.unlock();
    mVideoLock.unlock();
    LOGI(TAG, "CoreFlow : flushBuffer end");
}

void OutputSource::flushVideoBuffer() {
    if (videoPacketQueue.size() > 0) {
        videoPacketQueue.clear();
    }
}

void OutputSource::flushAudioBuffer() {
    if (audioPacketQueue.size() > 0) {
        audioPacketQueue.clear();
    }
}

uint32_t OutputSource::getAudioBufferSize() {
    return static_cast<uint32_t>(audioPacketQueue.size());
}

uint32_t OutputSource::getVideoBufferSize() {
    return static_cast<uint32_t>(videoPacketQueue.size());
}
