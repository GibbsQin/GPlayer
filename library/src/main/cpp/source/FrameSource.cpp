/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#include <base/Log.h>
#include <media/MediaHelper.h>
#include <thread>
#include "FrameSource.h"

#define TAG "FrameSource"

FrameSource::FrameSource(int audioMaxSize, int videoMaxSize) {
    LOGI(TAG, "CoreFlow : create FrameSource");
    audioPacketQueue = new ConcurrentQueue<MediaData*>(audioMaxSize, "AudioFrameQueue");
    videoPacketQueue = new ConcurrentQueue<MediaData*>(videoMaxSize, "VideoFrameQueue");
}

FrameSource::~FrameSource() {
    LOGI(TAG, "CoreFlow : FrameSource destroyed %d %d",
         audioPacketQueue->size(), videoPacketQueue->size());
    delete audioPacketQueue;
    audioPacketQueue = nullptr;
    delete videoPacketQueue;
    videoPacketQueue = nullptr;
}

unsigned long FrameSource::pushAudFrame(MediaData *frame) {
    LOGD(TAG, "pushAudFrame %lld", frame->pts);
    auto desBuffer = new MediaData();
    MediaHelper::copy(frame, desBuffer);
    return audioPacketQueue->push(desBuffer);
}

unsigned long FrameSource::pushVidFrame(MediaData *frame) {
    LOGD(TAG, "pushVidFrame %lld", frame->pts);
    auto desBuffer = new MediaData();
    MediaHelper::copy(frame, desBuffer);
    return videoPacketQueue->push(desBuffer);
}

unsigned long FrameSource::readAudFrame(MediaData **frame) {
    unsigned long size = audioPacketQueue->front(frame);
    if (size > 0) {
        LOGD(TAG, "readAudFrame %lld", (*frame)->pts);
    }
    return size;
}

unsigned long FrameSource::readVidFrame(MediaData **frame) {
    unsigned long size = videoPacketQueue->front(frame);
    if (size > 0) {
        LOGD(TAG, "readVidFrame %lld", (*frame)->pts);
    }
    return size;
}

void FrameSource::popAudFrame(MediaData *frame) {
    LOGD(TAG, "popAudFrame %lld", frame->pts);
    audioPacketQueue->pop();
}

void FrameSource::popVidFrame(MediaData *frame) {
    LOGD(TAG, "popVidFrame %lld", frame->pts);
    videoPacketQueue->pop();
}

void FrameSource::flush() {
    audioPacketQueue->flush();
    videoPacketQueue->flush();
    LOGI(TAG, "flushBuffer");
}

void FrameSource::reset() {
    audioPacketQueue->reset();
    videoPacketQueue->reset();
}

unsigned long FrameSource::audioSize() {
    return audioPacketQueue->size();
}

unsigned long FrameSource::videoSize() {
    return videoPacketQueue->size();
}
