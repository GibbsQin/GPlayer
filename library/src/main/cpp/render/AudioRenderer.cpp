//
// Created by Gibbs on 2020/12/20.
//

#include <base/Log.h>
#include "AudioRenderer.h"

AudioRenderer::AudioRenderer(FrameSource *source) {
    frameSource = source;
    audioTrackJni = nullptr;
}

AudioRenderer::~AudioRenderer() = default;

void AudioRenderer::setAudioTrack(AudioTrackJni *track) {
    audioTrackJni = track;
}

void AudioRenderer::setAudioParams(int rate, int count, int f, int bytes) {
    this->sampleRate = rate;
    this->channels = count;
    this->format = f;
    this->bytesPerSample = bytes;
}

void AudioRenderer::init() {
    audioTrackJni->start(sampleRate, format, channels, bytesPerSample);
}

uint64_t AudioRenderer::render(uint64_t nowMs) {
    MediaData *mediaData;
    if (frameSource->readAudioBuffer(&mediaData) > 0) {
        LOGI("AudioRenderer", "render audio %lld", mediaData->pts);
        audioTrackJni->write(mediaData->data, mediaData->size);
        frameSource->popAudioBuffer();
        return mediaData->pts;
    }
    return -1;
}

void AudioRenderer::release() {
    audioTrackJni->stop();
    delete audioTrackJni;
    audioTrackJni = nullptr;
}
