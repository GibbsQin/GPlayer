#include <media/MediaInfoJni.h>
#include <media/MediaDataJni.h>
#include <base/Log.h>
#include <utils/JniHelper.h>
#include <unistd.h>
#include "MediaSource.h"

#define TAG "MediaSourceC"

MediaSource::MediaSource() {
    LOGI(TAG, "CoreFlow : create MediaSource");
    mAVHeader = new MediaInfo();
}

MediaSource::~MediaSource() {
    flushBuffer();
    delete mAVHeader;
    LOGE(TAG, "CoreFlow : MediaSource destroyed");
}

void MediaSource::onInit(MediaInfo *header) {
    mAVHeader = header;
    LOGI(TAG, "CoreFlow : onInit header videoType:%d, videoWidth:%d, videoHeight:%d, videoFrameRate:%d,"
              "audioType:%d, audioMode:%d, audioBitWidth:%d, audioSampleRate:%d, sampleNumPerFrame:%d",
         header->videoType, header->videoWidth, header->videoHeight, header->videoFrameRate,
         header->audioType, header->audioMode, header->audioBitWidth, header->audioSampleRate,
         header->sampleNumPerFrame);
}

uint32_t MediaSource::onReceiveAudio(MediaData *inPacket) {
    uint32_t queueSize = 0;
    audioPacketQueue.push(inPacket);
    queueSize = audioPacketQueue.size();
    return queueSize;
}

uint32_t MediaSource::onReceiveVideo(MediaData *inPacket) {
    uint32_t queueSize = 0;
    videoPacketQueue.push(inPacket);
    queueSize = videoPacketQueue.size();
    return queueSize;
}

void MediaSource::onRelease() {
    LOGE(TAG, "CoreFlow : onRelease");
}

void MediaSource::flushBuffer() {
    LOGI(TAG, "flushBuffer start");
    mVideoLock.lock();
    mAudioLock.lock();
    flushAudioBuffer();
    flushVideoBuffer();
    mAudioLock.unlock();
    mVideoLock.unlock();
    LOGI(TAG, "flushBuffer end");
}

void MediaSource::flushVideoBuffer() {
    while (!videoPacketQueue.empty()) {
        MediaData *videoItem = videoPacketQueue.front();
        if (videoItem) {
            free(videoItem->data);
            delete videoItem;
        } else {
            LOGE(TAG, "videoItem is null");
        }
        videoPacketQueue.pop();
    }
}

void MediaSource::flushAudioBuffer() {
    while (!audioPacketQueue.empty()) {
        MediaData *audioItem = audioPacketQueue.front();
        if (audioItem) {
            free(audioItem->data);
            delete audioItem;
        } else {
            LOGE(TAG, "audioItem is null");
        }
        audioPacketQueue.pop();
    }
}

MediaInfo *MediaSource::getAVHeader() {
    return mAVHeader;
}

int MediaSource::readAudioBuffer(MediaData **avData) {
    if (audioPacketQueue.empty()) {
        return AV_SOURCE_EMPTY;
    }
    *avData = audioPacketQueue.front();
    if (!(*avData)) {
        LOGE(TAG, "readAudioBuffer item is null");
        audioPacketQueue.pop();
        return 0;
    }
    return audioPacketQueue.size();
}

int MediaSource::readVideoBuffer(MediaData **avData) {
    if (videoPacketQueue.empty()) {
        return AV_SOURCE_EMPTY;
    }
    *avData = videoPacketQueue.front();
    if (!(*avData)) {
        LOGE(TAG, "readVideoBuffer item is null");
        videoPacketQueue.pop();
        return 0;
    }
    return videoPacketQueue.size();
}

void MediaSource::popAudioBuffer() {
    mAudioLock.lock();
    audioPacketQueue.pop();
    mAudioLock.unlock();
}

void MediaSource::popVideoBuffer() {
    mVideoLock.lock();
    videoPacketQueue.pop();
    mVideoLock.unlock();
}
