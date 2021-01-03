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
    demuxerHelper = nullptr;
    decoderHelper = nullptr;
    renderHelper = nullptr;
    messageHelper = new MessageHelper(messageQueue, obj);
    demuxerThread = nullptr;
    audioDecodeThread = nullptr;
    videoDecodeThread = nullptr;
    audioRenderThread = nullptr;
    videoRenderThread = nullptr;
    messageThread = nullptr;
    mFlags = flag;
}

GPlayer::~GPlayer() {
    delete audioRenderThread;
    delete videoRenderThread;
    delete audioDecodeThread;
    delete videoDecodeThread;
    delete demuxerThread;
    delete messageThread;
    delete decoderHelper;
    delete demuxerHelper;
    delete renderHelper;
    delete messageHelper;
    delete outputSource;
    delete inputSource;
    delete messageQueue;
    LOGI(TAG, "CoreFlow : GPlayerImp destroyed");
}

void GPlayer::setSurface(ANativeWindow *window) {
    nativeWindow = window;
    LOGI(TAG, "setSurface %p", nativeWindow);
}

void GPlayer::setAudioTrack(AudioTrackJni *track) {
    audioTrackJni = track;
    LOGI(TAG, "setAudioTrack %p", audioTrackJni);
}

void GPlayer::prepare(const std::string &url) {
    LOGI(TAG, "CoreFlow : prepare %s", url.c_str());
    startMessageLoop();
    startDemuxing(url);
}

void GPlayer::start() {
    LOGI(TAG, "CoreFlow : start");
    startDecoding();
    startRendering();
}

void GPlayer::pause() {
    audioRenderThread->pause();
    videoRenderThread->pause();
    audioDecodeThread->pause();
    videoDecodeThread->pause();
    demuxerThread->pause();
    messageThread->pause();
}

void GPlayer::resume() {
    audioRenderThread->resume();
    videoRenderThread->resume();
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
    stopDecoding();
    stopRendering();
    stopMessageLoop();

    outputSource->flush();
    inputSource->flush();
    messageQueue->flush();

    messageHelper->notifyJava(MSG_DOMAIN_STATE, STATE_STOPPED, 0, nullptr, nullptr);
}

void GPlayer::startMessageLoop() {
    LOGI(TAG, "CoreFlow : startMessageLoop");
    messageThread = LoopThreadHelper::createLoopThread(
            std::bind(&MessageHelper::processMessage, messageHelper, std::placeholders::_1, std::placeholders::_2));
}

void GPlayer::stopMessageLoop() {
    messageQueue->reset();
    LoopThreadHelper::destroyThread(&messageThread);
    LOGI(TAG, "CoreFlow : message thread was stopped!");
}

void GPlayer::startDemuxing(const std::string &url) {
    LOGI(TAG, "CoreFlow : startDemuxing");
    demuxerHelper = new DemuxerHelper(url, inputSource, messageQueue);
    demuxerThread = LoopThreadHelper::createLoopThread(
            MAX_BUFFER_PACKET_SIZE,
            std::bind(&DemuxerHelper::init, demuxerHelper),
            std::bind(&DemuxerHelper::readPacket, demuxerHelper, std::placeholders::_1, std::placeholders::_2),
            std::bind(&DemuxerHelper::release, demuxerHelper));
}

void GPlayer::stopDemuxing() {
    LoopThreadHelper::destroyThread(&demuxerThread);
    delete demuxerHelper;
    demuxerHelper = nullptr;
    LOGI(TAG, "CoreFlow : demuxing thread was stopped!");
}

void GPlayer::startDecoding() {
    LOGI(TAG, "CoreFlow : startDecode");
    int mediaCodecFlag = (mFlags & AV_FLAG_SOURCE_MEDIA_CODEC) == AV_FLAG_SOURCE_MEDIA_CODEC;
    decoderHelper = new DecoderHelper(inputSource, outputSource, messageQueue);
    decoderHelper->setMediaCodec(static_cast<bool>(mediaCodecFlag));
    if (decoderHelper->onInit() < 0) {
        messageHelper->notifyJava(MSG_DOMAIN_ERROR, 1, 0, const_cast<char *>("not support this codec"), nullptr);
        return;
    }

    audioDecodeThread = LoopThreadHelper::createLoopThread(MAX_BUFFER_FRAME_SIZE,
            std::bind(&DecoderHelper::processAudioBuffer, decoderHelper, std::placeholders::_1, std::placeholders::_2));

    videoDecodeThread = LoopThreadHelper::createLoopThread(MAX_BUFFER_FRAME_SIZE,
            std::bind(&DecoderHelper::processVideoBuffer, decoderHelper, std::placeholders::_1, std::placeholders::_2));
}

void GPlayer::stopDecoding() {
    LoopThreadHelper::destroyThread(&audioDecodeThread);
    LoopThreadHelper::destroyThread(&videoDecodeThread);
    if (decoderHelper != nullptr) {
        decoderHelper->onRelease();
        delete decoderHelper;
        decoderHelper = nullptr;
    }
    LOGI(TAG, "CoreFlow : decoding threads were stopped!");
}

void GPlayer::startRendering() {
    LOGI(TAG, "CoreFlow : startRender");
    int width = inputSource->getVideoAVCodecParameters()->width;
    int height = inputSource->getVideoAVCodecParameters()->height;
    int sampleRate = inputSource->getAudioAVCodecParameters()->sample_rate;
    int channels = inputSource->getAudioAVCodecParameters()->channels;
    int format = inputSource->getAudioAVCodecParameters()->format;
    int bytesPerSample = inputSource->getAudioAVCodecParameters()->frame_size;
    renderHelper = new RenderHelper(outputSource, messageQueue);
    renderHelper->setSurface(nativeWindow);
    renderHelper->setAudioTrack(audioTrackJni);
    renderHelper->setVideoParams(width, height);
    renderHelper->setAudioParams(sampleRate, channels, format, bytesPerSample);
    audioRenderThread = LoopThreadHelper::createLoopThread(
            MAX_VALUE,
            std::bind(&RenderHelper::initAudioRenderer, renderHelper),
            std::bind(&RenderHelper::renderAudio, renderHelper, std::placeholders::_1, std::placeholders::_2),
            std::bind(&RenderHelper::releaseAudioRenderer, renderHelper));

    videoRenderThread = LoopThreadHelper::createLoopThread(
            MAX_VALUE,
            std::bind(&RenderHelper::initVideoRenderer, renderHelper),
            std::bind(&RenderHelper::renderVideo, renderHelper, std::placeholders::_1, std::placeholders::_2),
            std::bind(&RenderHelper::releaseVideoRenderer, renderHelper));
}

void GPlayer::stopRendering() {
    LoopThreadHelper::destroyThread(&audioRenderThread);
    LoopThreadHelper::destroyThread(&videoRenderThread);
    delete renderHelper;
    renderHelper = nullptr;
    LOGI(TAG, "CoreFlow : render threads were stopped!");
}
