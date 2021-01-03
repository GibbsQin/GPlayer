/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#include <base/Log.h>
#include <thread>
#include "AudioRenderer.h"

AudioRenderer::AudioRenderer(FrameSource *source) {
    frameSource = source;
    audioTrackJni = nullptr;
}

AudioRenderer::~AudioRenderer() = default;

void AudioRenderer::setAudioTrack(AudioTrackJni *track) {
    audioTrackJni = track;
}

void AudioRenderer::setAudioParams(int rate, int count, int fmt, int bytes) {
    this->sampleRate = rate;
    this->channels = count;
    this->format = fmt;
    this->bytesPerSample = bytes;
}

void AudioRenderer::init() {
    audioTrackJni->start(sampleRate, format, channels, bytesPerSample);
}

uint64_t AudioRenderer::render(uint64_t nowMs) {
    MediaData *mediaData;
    if (frameSource->readAudFrame(&mediaData) > 0) {
        audioTrackJni->write(mediaData->data, mediaData->size);
        frameSource->popAudFrame(mediaData);
        return mediaData->pts;
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return 0;
}

void AudioRenderer::release() {
    audioTrackJni->stop();
    delete audioTrackJni;
    audioTrackJni = nullptr;
}
