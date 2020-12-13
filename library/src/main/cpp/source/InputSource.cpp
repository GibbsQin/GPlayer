#include <base/Log.h>
#include "InputSource.h"

#define TAG "InputSource"

InputSource::InputSource() {
    LOGI(TAG, "CoreFlow : create InputSource");
    mFormatInfo = static_cast<FormatInfo *>(malloc(sizeof(FormatInfo)));
    mFormatInfo->audcodecpar = avcodec_parameters_alloc();
    mFormatInfo->vidcodecpar = avcodec_parameters_alloc();
    mFormatInfo->subcodecpar = avcodec_parameters_alloc();
}

InputSource::~InputSource() {
    LOGI(TAG, "CoreFlow : InputSource destroyed %d %d",
         audioPacketQueue.size(), videoPacketQueue.size());
    avcodec_parameters_free(&mFormatInfo->audcodecpar);
    avcodec_parameters_free(&mFormatInfo->vidcodecpar);
    avcodec_parameters_free(&mFormatInfo->subcodecpar);
    free(mFormatInfo);
}

void InputSource::queueInfo(FormatInfo *formatInfo) {
    mFormatInfo = formatInfo;
}

uint32_t InputSource::queueAudPkt(AVPacket *pkt, AVRational time_base) {
    uint32_t queueSize = 0;
    AVPacket *packet = av_packet_alloc();
    av_packet_ref(packet, pkt);
    packet->pts = ffmpeg_pts2timeus(time_base, pkt->pts);
    packet->dts = ffmpeg_pts2timeus(time_base, pkt->dts);
    audioPacketQueue.push_back(packet);
    queueSize = static_cast<uint32_t>(audioPacketQueue.size());
    LOGI(TAG, "queue audio packet %lld", packet->pts);
    return queueSize;
}

uint32_t InputSource::queueVidPkt(AVPacket *pkt, AVRational time_base) {
    uint32_t queueSize = 0;
    AVPacket *packet = av_packet_alloc();
    av_packet_ref(packet, pkt);
    packet->pts = ffmpeg_pts2timeus(time_base, pkt->pts);
    packet->dts = ffmpeg_pts2timeus(time_base, pkt->dts);
    videoPacketQueue.push_back(packet);
    queueSize = static_cast<uint32_t>(videoPacketQueue.size());
    LOGI(TAG, "queue video packet %lld", packet->pts);
    return queueSize;
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

int InputSource::dequeAudPkt(AVPacket **pkt) {
    if (audioPacketQueue.size() <= 0) {
        return AV_SOURCE_EMPTY;
    }
    *pkt = audioPacketQueue.front();
    return static_cast<int>(audioPacketQueue.size());
}

int InputSource::dequeVidPkt(AVPacket **pkt) {
    if (videoPacketQueue.size() <= 0) {
        return AV_SOURCE_EMPTY;
    }
    *pkt = videoPacketQueue.front();
    return static_cast<int>(videoPacketQueue.size());
}

void InputSource::popAudPkt(AVPacket *pkt) {
    av_packet_free(&pkt);
    mAudioLock.lock();
    if (audioPacketQueue.size() > 0) {
        audioPacketQueue.pop_front();
        LOGI(TAG, "pop audio packet");
    }
    mAudioLock.unlock();
}

void InputSource::popVidPkt(AVPacket *pkt) {
    av_packet_free(&pkt);
    mVideoLock.lock();
    if (videoPacketQueue.size() > 0) {
        videoPacketQueue.pop_front();
        LOGI(TAG, "pop video packet");
    }
    mVideoLock.unlock();
}

void InputSource::flush() {
    mVideoLock.lock();
    mAudioLock.lock();
    flushAudioBuffer();
    flushVideoBuffer();
    mAudioLock.unlock();
    mVideoLock.unlock();
    LOGI(TAG, "CoreFlow : flushBuffer");
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

uint32_t InputSource::getAudSize() {
    return static_cast<uint32_t>(audioPacketQueue.size());
}

uint32_t InputSource::getVidSize() {
    return static_cast<uint32_t>(videoPacketQueue.size());
}
