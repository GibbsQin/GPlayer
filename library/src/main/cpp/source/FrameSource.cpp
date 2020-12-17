#include <media/MediaDataJni.h>
#include <base/Log.h>
#include <media/MediaHelper.h>
#include "FrameSource.h"

#define TAG "OutputSource"

FrameSource::FrameSource() {
    LOGI(TAG, "CoreFlow : create OutputSource");
}

FrameSource::~FrameSource() {
    LOGI(TAG, "CoreFlow : OutputSource destroyed %d %d",
         audioPacketQueue.size(), videoPacketQueue.size());
}

uint32_t FrameSource::onReceiveAudio(MediaData *inPacket) {
    auto desBuffer = new MediaData();
    MediaHelper::copy(inPacket, desBuffer);
    audioPacketQueue.push_back(desBuffer);
    auto queueSize = static_cast<uint32_t>(audioPacketQueue.size());
    LOGI(TAG, "queue audio packet %lld", desBuffer->pts);
    return queueSize;
}

uint32_t FrameSource::onReceiveVideo(MediaData *inPacket) {
    auto desBuffer = new MediaData();
    MediaHelper::copy(inPacket, desBuffer);
    videoPacketQueue.push_back(desBuffer);
    auto queueSize = static_cast<uint32_t>(videoPacketQueue.size());
    LOGI(TAG, "queue video packet %lld", desBuffer->pts);
    return queueSize;
}

int FrameSource::readAudioBuffer(MediaData **avData) {
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

int FrameSource::readVideoBuffer(MediaData **avData) {
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

void FrameSource::popAudioBuffer() {
    mAudioLock.lock();
    if (audioPacketQueue.size() > 0) {
        MediaData *mediaData = audioPacketQueue.front();
        delete mediaData;
        audioPacketQueue.pop_front();
        LOGI(TAG, "pop audio packet");
    }
    mAudioLock.unlock();
}

void FrameSource::popVideoBuffer() {
    mVideoLock.lock();
    if (videoPacketQueue.size() > 0) {
        MediaData *mediaData = videoPacketQueue.front();
        delete mediaData;
        videoPacketQueue.pop_front();
        LOGI(TAG, "pop video packet");
    }
    mVideoLock.unlock();
}

void FrameSource::flushBuffer() {
    mVideoLock.lock();
    mAudioLock.lock();
    flushAudioBuffer();
    flushVideoBuffer();
    mAudioLock.unlock();
    mVideoLock.unlock();
    LOGI(TAG, "CoreFlow : flushBuffer");
}

void FrameSource::flushVideoBuffer() {
    if (videoPacketQueue.size() > 0) {
        videoPacketQueue.clear();
    }
}

void FrameSource::flushAudioBuffer() {
    if (audioPacketQueue.size() > 0) {
        audioPacketQueue.clear();
    }
}

uint32_t FrameSource::getAudioBufferSize() {
    return static_cast<uint32_t>(audioPacketQueue.size());
}

uint32_t FrameSource::getVideoBufferSize() {
    return static_cast<uint32_t>(videoPacketQueue.size());
}