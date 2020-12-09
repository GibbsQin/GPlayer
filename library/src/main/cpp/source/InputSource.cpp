#include <base/Log.h>
#include "InputSource.h"

#define TAG "MediaSourceC"

InputSource::InputSource() {
    LOGI(TAG, "CoreFlow : create MediaSource");
    mFormatInfo = static_cast<FormatInfo *>(malloc(sizeof(FormatInfo)));
    mFormatInfo->audcodecpar = avcodec_parameters_alloc();
    mFormatInfo->vidcodecpar = avcodec_parameters_alloc();
    mFormatInfo->subcodecpar = avcodec_parameters_alloc();
}

InputSource::~InputSource() {
    LOGI(TAG, "CoreFlow : MediaSource destroyed %d %d",
         audioPacketQueue.size(), videoPacketQueue.size());
    avcodec_parameters_free(&mFormatInfo->audcodecpar);
    avcodec_parameters_free(&mFormatInfo->vidcodecpar);
    avcodec_parameters_free(&mFormatInfo->subcodecpar);
    free(mFormatInfo);
}

void InputSource::onInit(FormatInfo *formatInfo) {
    mFormatInfo = formatInfo;
}

uint32_t InputSource::onReceiveAudio(AVPacket *pkt) {
    uint32_t queueSize = 0;
    audioPacketQueue.push_back(pkt);
    queueSize = static_cast<uint32_t>(audioPacketQueue.size());
    return queueSize;
}

uint32_t InputSource::onReceiveVideo(AVPacket *pkt) {
    uint32_t queueSize = 0;
    videoPacketQueue.push_back(pkt);
    queueSize = static_cast<uint32_t>(videoPacketQueue.size());
    return queueSize;
}

void InputSource::onRelease() {
}

FormatInfo *InputSource::getFormatInfo() {
    return mFormatInfo;
}

AVCodecParameters *InputSource::getAudioAVCodecParameters() {
    return mFormatInfo->audcodecpar;
}

AVCodecParameters *InputSource::getVideoAVCodecParameters() {
    return mFormatInfo->vidcodecpar;
}

AVCodecParameters *InputSource::getSubtitleAVCodecParameters() {
    return mFormatInfo->subcodecpar;
}

int InputSource::readAudioBuffer(AVPacket **pkt) {
    if (audioPacketQueue.size() <= 0) {
        return AV_SOURCE_EMPTY;
    }
    *pkt = audioPacketQueue.front();
    return static_cast<int>(audioPacketQueue.size());
}

int InputSource::readVideoBuffer(AVPacket **pkt) {
    if (videoPacketQueue.size() <= 0) {
        return AV_SOURCE_EMPTY;
    }
    *pkt = videoPacketQueue.front();
    return static_cast<int>(videoPacketQueue.size());
}

void InputSource::popAudioBuffer() {
    mAudioLock.lock();
    if (audioPacketQueue.size() > 0) {
        audioPacketQueue.pop_front();
    }
    mAudioLock.unlock();
}

void InputSource::popVideoBuffer() {
    mVideoLock.lock();
    if (videoPacketQueue.size() > 0) {
        videoPacketQueue.pop_front();
    }
    mVideoLock.unlock();
}

void InputSource::flushBuffer() {
    LOGI(TAG, "CoreFlow : flushBuffer start");
    mVideoLock.lock();
    mAudioLock.lock();
    flushAudioBuffer();
    flushVideoBuffer();
    mAudioLock.unlock();
    mVideoLock.unlock();
    LOGI(TAG, "CoreFlow : flushBuffer end");
}

void InputSource::flushVideoBuffer() {
    if (videoPacketQueue.size() > 0) {
        videoPacketQueue.clear();
    }
}

void InputSource::flushAudioBuffer() {
    if (audioPacketQueue.size() > 0) {
        audioPacketQueue.clear();
    }
}

uint32_t InputSource::getAudioBufferSize() {
    return static_cast<uint32_t>(audioPacketQueue.size());
}

uint32_t InputSource::getVideoBufferSize() {
    return static_cast<uint32_t>(videoPacketQueue.size());
}
