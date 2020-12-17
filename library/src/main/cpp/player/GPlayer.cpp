//
// Created by Gibbs on 2020/7/16.
//

#include "GPlayer.h"
#include "media/MediaData.h"
#include <base/Log.h>
#include <cstdint>
#include <unistd.h>

extern "C" {
#include <demuxing/avformat_def.h>
}

#define TAG "GPlayerC"

GPlayer::GPlayer(uint32_t flag, jobject obj) {
    outputSource = new FrameSource();
    inputSource = new PacketSource();
    playerJni = new GPlayerJni(obj);
    messageQueue = new MessageQueue();
    audioDecodeThread = nullptr;
    videoDecodeThread = nullptr;
    demuxerThread = nullptr;
    mFlags = flag;
    startMessageLoop();
    LOGI(TAG, "CoreFlow : start demuxing flag %d", mFlags);
}

GPlayer::~GPlayer() {
    stopMessageLoop();
    if (audioDecodeThread) {
        delete audioDecodeThread;
        audioDecodeThread = nullptr;
    }
    if (videoDecodeThread) {
        delete videoDecodeThread;
        videoDecodeThread = nullptr;
    }
    if (demuxerThread) {
        delete demuxerThread;
        demuxerThread = nullptr;
    }
    if (messageThread) {
        delete messageThread;
        messageThread = nullptr;
    }
    if (outputSource) {
        delete outputSource;
        outputSource = nullptr;
    }
    if (inputSource) {
        delete inputSource;
        inputSource = nullptr;
    }
    if (decoderHelper) {
        delete decoderHelper;
        decoderHelper = nullptr;
    }
    if (mUrl) {
        free(mUrl);
    }
    playerJni->onMessageCallback(MSG_JAVA_STATE, STATE_RELEASED, 0, nullptr, nullptr);
    if (playerJni) {
        delete playerJni;
        playerJni = nullptr;
    }
    LOGI(TAG, "CoreFlow : GPlayerImp destroyed");
}

void GPlayer::prepare(const std::string &url) {
    mUrl = static_cast<char *>(malloc(url.length() + 1));
    memcpy(mUrl, url.c_str(), url.length());
    mUrl[url.length()] = '\0';

    startDemuxing();
}

void GPlayer::start() {
    startDecode();
}

void GPlayer::pause() {
    audioDecodeThread->pause();
    videoDecodeThread->pause();
}

void GPlayer::resume() {
    audioDecodeThread->resume();
    videoDecodeThread->resume();
}

void GPlayer::seekTo(uint32_t secondUs) {
    mSeekUs = secondUs;
}

void GPlayer::stop() {
    stopDemuxing();
    LOGI(TAG, "CoreFlow : demuxing thread is stopped!");
    stopDecode();
    LOGI(TAG, "CoreFlow : decode threads were stopped!");
    inputSource->flush();
    playerJni->onMessageCallback(MSG_JAVA_STATE, STATE_STOPPED, 0, nullptr, nullptr);
}

void GPlayer::setFlags(uint32_t flags) {
    mFlags = flags;
    int mediaCodecFlag = (mFlags & AV_FLAG_SOURCE_MEDIA_CODEC) == AV_FLAG_SOURCE_MEDIA_CODEC;
    if (mediaCodecFlag) {
        decoderHelper->enableMediaCodec();
    }
}

void GPlayer::startMessageLoop() {
    messageThread = new LoopThread();
    messageThread->setUpdateFunc(std::bind(&GPlayer::processMessage, this,
                                           std::placeholders::_1, std::placeholders::_2));
    messageThread->start();
}

void GPlayer::stopMessageLoop() {
    if (messageThread && messageThread->hasStarted()) {
        messageThread->stop();
        messageThread->join();
    }
}

int GPlayer::processMessage(int arg1, long arg2) {
    Message *message = static_cast<Message *>(malloc(sizeof(Message)));
    messageQueue->dequeMessage(message);
    LOGI(TAG, "processMessage %d, %d. %lld", message->from, message->type, message->extra);
    return 10;
}

void GPlayer::startDecode() {
    LOGI(TAG, "CoreFlow : startDecode");
    int mediaCodecFlag = (mFlags & AV_FLAG_SOURCE_MEDIA_CODEC) == AV_FLAG_SOURCE_MEDIA_CODEC;
    decoderHelper = new DecoderHelper(inputSource, outputSource, messageQueue, static_cast<bool>(mediaCodecFlag));
    int ret = decoderHelper->onInit();
    if (ret < 0) {
        playerJni->onMessageCallback(MSG_JAVA_ERROR, 1, 0,
                                     const_cast<char *>("not support this codec"), nullptr);
        return;
    }

    audioDecodeThread = new LoopThread();
    audioDecodeThread->setUpdateFunc(std::bind(&DecoderHelper::processAudioBuffer, decoderHelper,
            std::placeholders::_1, std::placeholders::_2));
    audioDecodeThread->start();

    videoDecodeThread = new LoopThread();
    videoDecodeThread->setUpdateFunc(std::bind(&DecoderHelper::processVideoBuffer, decoderHelper,
            std::placeholders::_1, std::placeholders::_2));
    videoDecodeThread->start();
}

void GPlayer::stopDecode() {
    if (audioDecodeThread && audioDecodeThread->hasStarted()) {
        audioDecodeThread->stop();
        audioDecodeThread->join();
    }
    if (videoDecodeThread && videoDecodeThread->hasStarted()) {
        videoDecodeThread->stop();
        videoDecodeThread->join();
    }
    decoderHelper->onRelease();
    decoderHelper = nullptr;
}

void GPlayer::startDemuxing() {
    demuxerHelper = new DemuxerHelper(mUrl, inputSource, messageQueue);
    demuxerThread = new LoopThread();
    demuxerThread->setStartFunc(std::bind(&DemuxerHelper::init, demuxerHelper));
    demuxerThread->setUpdateFunc(std::bind(&DemuxerHelper::update, demuxerHelper,
            std::placeholders::_1, std::placeholders::_2));
    demuxerThread->setEndFunc(std::bind(&DemuxerHelper::release, demuxerHelper));
    demuxerThread->start();
}

void GPlayer::stopDemuxing() {
    if (demuxerThread && demuxerThread->hasStarted()) {
        demuxerThread->stop();
        demuxerThread->join();
    }
    decoderHelper = nullptr;
}

PacketSource *GPlayer::getInputSource() {
    return inputSource;
}

FrameSource *GPlayer::getOutputSource() {
    return outputSource;
}
