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
    playerJni->onMessageCallback(MSG_FROM_STATE, STATE_RELEASED, 0, nullptr, nullptr);
    if (playerJni) {
        delete playerJni;
        playerJni = nullptr;
    }
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
    playerJni->onMessageCallback(MSG_FROM_STATE, STATE_STOPPED, 0, nullptr, nullptr);
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
    auto message = static_cast<Message *>(malloc(sizeof(Message)));
    if (messageQueue->dequeMessage(message) < 0) {
        return 0;
    }
    LOGI(TAG, "processMessage %d, %d, %lld", message->from, message->type, message->extra);
    if (message->from == MSG_FROM_STATE) {
        playerJni->onMessageCallback(MSG_FROM_STATE, message->type, message->extra, nullptr, nullptr);
    } else if (message->from == MSG_FROM_SIZE) {
        playerJni->onMessageCallback(MSG_FROM_SIZE, message->type, message->extra, nullptr, nullptr);
    } else if (message->from == MSG_FROM_ERROR) {
        if (message->type == MSG_DEMUXING_ERROR) {
            playerJni->onMessageCallback(MSG_FROM_ERROR, message->type, message->extra,
                    av_err2str(message->extra), nullptr);
        }
    }
    free(message);
    return 0;
}

void GPlayer::startDemuxing(const std::string &url) {
    demuxerHelper = new DemuxerHelper(url, inputSource, messageQueue);
    demuxerThread = new LoopThread(MAX_BUFFER_SIZE);
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
    delete demuxerHelper;
    demuxerHelper = nullptr;
}

void GPlayer::startDecode() {
    LOGI(TAG, "CoreFlow : startDecode");
    int mediaCodecFlag = (mFlags & AV_FLAG_SOURCE_MEDIA_CODEC) == AV_FLAG_SOURCE_MEDIA_CODEC;
    decoderHelper = new DecoderHelper(inputSource, outputSource, messageQueue, static_cast<bool>(mediaCodecFlag));
    int ret = decoderHelper->onInit();
    if (ret < 0) {
        playerJni->onMessageCallback(MSG_FROM_ERROR, 1, 0,
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
    delete decoderHelper;
    decoderHelper = nullptr;
}
