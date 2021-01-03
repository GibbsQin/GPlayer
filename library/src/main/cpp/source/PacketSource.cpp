#include <base/Log.h>
#include "PacketSource.h"

#define TAG "PacketSource"

static uint64_t ffmpeg_pts2timeus(AVRational time_base, int64_t pts) {
    return (uint64_t) (av_q2d(time_base) * pts * 1000000);
}

PacketSource::PacketSource() {
    LOGI(TAG, "CoreFlow : create InputSource");
    mFormatInfo = static_cast<FormatInfo *>(malloc(sizeof(FormatInfo)));
    mFormatInfo->audcodecpar = avcodec_parameters_alloc();
    mFormatInfo->vidcodecpar = avcodec_parameters_alloc();
    mFormatInfo->subcodecpar = avcodec_parameters_alloc();
}

PacketSource::~PacketSource() {
    LOGI(TAG, "CoreFlow : InputSource destroyed %d %d",
         audioPacketQueue.size(), videoPacketQueue.size());
    avcodec_parameters_free(&mFormatInfo->audcodecpar);
    avcodec_parameters_free(&mFormatInfo->vidcodecpar);
    avcodec_parameters_free(&mFormatInfo->subcodecpar);
    free(mFormatInfo);
}

void PacketSource::queueInfo(FormatInfo *formatInfo) {
    mFormatInfo = formatInfo;
}

uint32_t PacketSource::queueAudPkt(AVPacket *pkt, AVRational time_base) {
    uint32_t queueSize = 0;
    AVPacket *packet = av_packet_alloc();
    av_packet_ref(packet, pkt);
    packet->pts = ffmpeg_pts2timeus(time_base, pkt->pts);
    packet->dts = ffmpeg_pts2timeus(time_base, pkt->dts);
    audioPacketQueue.push_back(packet);
    queueSize = static_cast<uint32_t>(audioPacketQueue.size());
    LOGD(TAG, "queue audio packet %lld", packet->pts);
    return queueSize;
}

uint32_t PacketSource::queueVidPkt(AVPacket *pkt, AVRational time_base) {
    uint32_t queueSize = 0;
    AVPacket *packet = av_packet_alloc();
    av_packet_ref(packet, pkt);
    packet->pts = ffmpeg_pts2timeus(time_base, pkt->pts);
    packet->dts = ffmpeg_pts2timeus(time_base, pkt->dts);
    videoPacketQueue.push_back(packet);
    queueSize = static_cast<uint32_t>(videoPacketQueue.size());
    LOGD(TAG, "queue video packet %lld", packet->pts);
    return queueSize;
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

int PacketSource::dequeAudPkt(AVPacket **pkt) {
    if (audioPacketQueue.size() <= 0) {
        return AV_SOURCE_EMPTY;
    }
    *pkt = audioPacketQueue.front();
    return static_cast<int>(audioPacketQueue.size());
}

int PacketSource::dequeVidPkt(AVPacket **pkt) {
    if (videoPacketQueue.size() <= 0) {
        return AV_SOURCE_EMPTY;
    }
    *pkt = videoPacketQueue.front();
    return static_cast<int>(videoPacketQueue.size());
}

void PacketSource::popAudPkt(AVPacket *pkt) {
    av_packet_free(&pkt);
    mAudioLock.lock();
    if (audioPacketQueue.size() > 0) {
        audioPacketQueue.pop_front();
        LOGD(TAG, "pop audio packet");
    }
    mAudioLock.unlock();
}

void PacketSource::popVidPkt(AVPacket *pkt) {
    av_packet_free(&pkt);
    mVideoLock.lock();
    if (videoPacketQueue.size() > 0) {
        videoPacketQueue.pop_front();
        LOGD(TAG, "pop video packet");
    }
    mVideoLock.unlock();
}

void PacketSource::flush() {
    mVideoLock.lock();
    mAudioLock.lock();
    flushAudioBuffer();
    flushVideoBuffer();
    mAudioLock.unlock();
    mVideoLock.unlock();
    LOGI(TAG, "CoreFlow : flushBuffer");
}

void PacketSource::flushVideoBuffer() {
    while (videoPacketQueue.size() > 0) {
        AVPacket *pkt = videoPacketQueue.front();
        av_packet_free(&pkt);
        videoPacketQueue.pop_front();
    }
}

void PacketSource::flushAudioBuffer() {
    while (audioPacketQueue.size() > 0) {
        AVPacket *pkt = audioPacketQueue.front();
        av_packet_free(&pkt);
        audioPacketQueue.pop_front();
    }
}
