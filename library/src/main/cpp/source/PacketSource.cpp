/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#include <base/Log.h>
#include <thread>
#include "PacketSource.h"

#define TAG "PacketSource"

static uint64_t ffmpeg_pts2timeus(AVRational time_base, int64_t pts) {
    return (uint64_t) (av_q2d(time_base) * pts * 1000000);
}

PacketSource::PacketSource(int audioMaxSize, int videoMaxSize) {
    LOGI(TAG, "CoreFlow : create PacketSource");
    mFormatInfo = static_cast<FormatInfo *>(malloc(sizeof(FormatInfo)));
    mFormatInfo->audcodecpar = avcodec_parameters_alloc();
    mFormatInfo->vidcodecpar = avcodec_parameters_alloc();
    mFormatInfo->subcodecpar = avcodec_parameters_alloc();

    audioPacketQueue = new AvConcurrentQueue(audioMaxSize, "AudioPacketQueue");
    videoPacketQueue = new AvConcurrentQueue(videoMaxSize, "VideoPacketQueue");
}

PacketSource::~PacketSource() {
    LOGI(TAG, "CoreFlow : PacketSource destroyed %d %d",
         audioPacketQueue->size(), videoPacketQueue->size());
    avcodec_parameters_free(&mFormatInfo->audcodecpar);
    avcodec_parameters_free(&mFormatInfo->vidcodecpar);
    avcodec_parameters_free(&mFormatInfo->subcodecpar);
    free(mFormatInfo);

    delete audioPacketQueue;
    audioPacketQueue = nullptr;
    delete videoPacketQueue;
    videoPacketQueue = nullptr;
}

void PacketSource::setFormatInfo(FormatInfo *formatInfo) {
    mFormatInfo = formatInfo;
}

FormatInfo *PacketSource::getFormatInfo() {
    return mFormatInfo;
}

AVCodecParameters *PacketSource::getAudioAVCodecParameters() {
    return mFormatInfo->audcodecpar;
}

AVCodecParameters *PacketSource::getVideoAVCodecParameters() {
    return mFormatInfo->vidcodecpar;
}

unsigned long PacketSource::pushAudPkt(AVPacket *pkt, AVRational time_base) {
    AVPacket *packet = av_packet_alloc();
    av_packet_ref(packet, pkt);
    packet->pts = ffmpeg_pts2timeus(time_base, pkt->pts);
    packet->dts = ffmpeg_pts2timeus(time_base, pkt->dts);
    LOGD(TAG, "pushAudPkt %lld", packet->pts);
    return audioPacketQueue->push(packet);
}

unsigned long PacketSource::pushVidPkt(AVPacket *pkt, AVRational time_base) {
    AVPacket *packet = av_packet_alloc();
    av_packet_ref(packet, pkt);
    packet->pts = ffmpeg_pts2timeus(time_base, pkt->pts);
    packet->dts = ffmpeg_pts2timeus(time_base, pkt->dts);
    LOGD(TAG, "pushVidPkt %lld", packet->pts);
    uint32_t size = videoPacketQueue->push(packet);
    return size;
}

unsigned long PacketSource::readAudPkt(AVPacket **pkt) {
    unsigned long size = audioPacketQueue->front(pkt);
    if (size > 0) {
        LOGD(TAG, "readAudPkt %lld", (*pkt)->pts);
    }
    return size;
}

unsigned long PacketSource::readVidPkt(AVPacket **pkt) {
    unsigned long size = videoPacketQueue->front(pkt);
    if (size > 0) {
        LOGD(TAG, "readVidPkt %lld", (*pkt)->pts);
    }
    return size;
}

void PacketSource::popAudPkt(AVPacket *pkt) {
    LOGD(TAG, "popAudPkt %lld", pkt->pts);
    audioPacketQueue->pop();
}

void PacketSource::popVidPkt(AVPacket *pkt) {
    LOGD(TAG, "popVidPkt %lld", pkt->pts);
    videoPacketQueue->pop();
}

void PacketSource::flush() {
    audioPacketQueue->flush();
    videoPacketQueue->flush();
    LOGI(TAG, "flushBuffer");
}

void PacketSource::reset() {
    audioPacketQueue->reset();
    videoPacketQueue->reset();
}

unsigned long PacketSource::audioSize() {
    return audioPacketQueue->size();
}

unsigned long PacketSource::videoSize() {
    return videoPacketQueue->size();
}