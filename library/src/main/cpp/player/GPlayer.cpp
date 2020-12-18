//
// Created by Gibbs on 2020/7/16.
//

#include "GPlayer.h"
#include "media/MediaData.h"
#include "LoopThreadHelper.h"
#include <base/Log.h>
#include <cstdint>
#include <unistd.h>

#define TAG "GPlayerC"

GPlayer::GPlayer(uint32_t flag, jobject obj) {
    outputSource = new FrameSource();
    inputSource = new PacketSource();
    messageQueue = new MessageQueue();
    messageHelper = new MessageHelper(messageQueue, obj);
    audioDecodeThread = nullptr;
    videoDecodeThread = nullptr;
    demuxerThread = nullptr;
    messageThread = nullptr;
    mFlags = flag;
    startMessageLoop();
}

GPlayer::~GPlayer() {
    stopMessageLoop();
    if (audioDecodeThread) delete audioDecodeThread;
    if (videoDecodeThread) delete videoDecodeThread;
    if (demuxerThread) delete demuxerThread;
    if (messageThread) delete messageThread;
    if (outputSource) delete outputSource;
    if (inputSource) delete inputSource;
    if (decoderHelper) delete decoderHelper;
    if (demuxerHelper) delete demuxerHelper;
    if (messageHelper) delete messageHelper;
    if (messageQueue) delete messageQueue;
    LOGI(TAG, "CoreFlow : GPlayerImp destroyed");
}

void GPlayer::prepare(const std::string &url) {
    LOGI(TAG, "CoreFlow : prepare %s", url.c_str());
    startDemuxing(url);
}

void GPlayer::start() {
    LOGI(TAG, "CoreFlow : start");
    startDecode();
}

void GPlayer::pause() {
    audioDecodeThread->pause();
    videoDecodeThread->pause();
    demuxerThread->pause();
    messageThread->pause();
}

void GPlayer::resume() {
    audioDecodeThread->resume();
    videoDecodeThread->resume();
    demuxerThread->resume();
    messageThread->resume();
}

void GPlayer::seekTo(uint32_t secondUs) {
    demuxerThread->setArgs(0, secondUs);
}

void GPlayer::stop() {
    LOGI(TAG, "CoreFlow : stop");
    stopDemuxing();
    LOGI(TAG, "CoreFlow : demuxing thread is stopped!");
    stopDecode();
    LOGI(TAG, "CoreFlow : decode threads were stopped!");
    inputSource->flush();
    messageQueue->flush();
    messageHelper->notifyJava(MSG_DOMAIN_STATE, STATE_STOPPED, 0, nullptr, nullptr);
}

void GPlayer::setFlags(uint32_t flags) {
    mFlags = flags;
}

PacketSource *GPlayer::getInputSource() {
    return inputSource;
}

FrameSource *GPlayer::getOutputSource() {
    return outputSource;
}

void GPlayer::startMessageLoop() {
    messageThread = LoopThreadHelper::createLoopThread(
            std::bind(&MessageHelper::processMessage, messageHelper, std::placeholders::_1, std::placeholders::_2));
}

void GPlayer::stopMessageLoop() {
    LoopThreadHelper::destroyThread(messageThread);
}

void GPlayer::startDemuxing(const std::string &url) {
    demuxerHelper = new DemuxerHelper(url, inputSource, messageQueue);
    demuxerThread = LoopThreadHelper::createLoopThread(
            MAX_BUFFER_PACKET_SIZE,
            std::bind(&DemuxerHelper::init, demuxerHelper),
            std::bind(&DemuxerHelper::readPacket, demuxerHelper, std::placeholders::_1, std::placeholders::_2),
            std::bind(&DemuxerHelper::release, demuxerHelper));
}

void GPlayer::stopDemuxing() {
    LoopThreadHelper::destroyThread(demuxerThread);
}

void GPlayer::startDecode() {
    LOGI(TAG, "CoreFlow : startDecode");
    int mediaCodecFlag = (mFlags & AV_FLAG_SOURCE_MEDIA_CODEC) == AV_FLAG_SOURCE_MEDIA_CODEC;
    decoderHelper = new DecoderHelper(inputSource, outputSource, messageQueue, static_cast<bool>(mediaCodecFlag));
    if (decoderHelper->onInit() < 0) {
        messageHelper->notifyJava(MSG_DOMAIN_ERROR, 1, 0, const_cast<char *>("not support this codec"), nullptr);
        return;
    }

    audioDecodeThread = LoopThreadHelper::createLoopThread(MAX_BUFFER_FRAME_SIZE,
            std::bind(&DecoderHelper::processAudioBuffer, decoderHelper, std::placeholders::_1, std::placeholders::_2));

    videoDecodeThread = LoopThreadHelper::createLoopThread(MAX_BUFFER_FRAME_SIZE,
            std::bind(&DecoderHelper::processVideoBuffer, decoderHelper, std::placeholders::_1, std::placeholders::_2));
}

void GPlayer::stopDecode() {
    LoopThreadHelper::destroyThread(audioDecodeThread);
    LoopThreadHelper::destroyThread(videoDecodeThread);
    decoderHelper->onRelease();
    delete decoderHelper;
    decoderHelper = nullptr;
}
