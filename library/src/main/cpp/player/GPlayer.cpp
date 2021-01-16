/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#include "GPlayer.h"
#include "media/MediaData.h"
#include "LoopThreadHelper.h"
#include <base/Log.h>
#include <cstdint>
#include <unistd.h>

#define TAG "GPlayerC"

GPlayer::GPlayer(uint32_t flag, jobject obj) {
    mFlags = flag;
    int mediaCodecFlag = (mFlags & AV_FLAG_SOURCE_MEDIA_CODEC) == AV_FLAG_SOURCE_MEDIA_CODEC;
    messageSource = new MessageSource();
    frameSource = new FrameSource(MAX_BUFFER_FRAME_SIZE, mediaCodecFlag ? 1 : MAX_BUFFER_FRAME_SIZE);
    packetSource = new PacketSource(MAX_BUFFER_PACKET_SIZE, MAX_BUFFER_PACKET_SIZE);
    demuxerHelper = nullptr;
    decoderHelper = nullptr;
    renderHelper = nullptr;
    messageHelper = new MessageHelper(messageSource, obj);
    demuxerThread = nullptr;
    audioDecodeThread = nullptr;
    videoDecodeThread = nullptr;
    audioRenderThread = nullptr;
    videoRenderThread = nullptr;
    messageThread = nullptr;
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
    delete frameSource;
    delete packetSource;
    delete messageSource;
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
    playState = -1;
    seekState = -1;
    bufferState = -1;
    isEof = false;
    onPlayStateChanged(STATE_PREPARING, 0);
    LOGI(TAG, "CoreFlow : prepare %s", url.c_str());
    startMessageLoop();
    startDemuxing(url);
}

void GPlayer::start() {
    LOGI(TAG, "CoreFlow : start playing");
    if (nativeWindow == nullptr) {
        messageHelper->notifyJava(MSG_DOMAIN_ERROR, MSG_ERROR_RENDERING, 0, "invalid surface", nullptr);
        return;
    }
    startDecoding();
    startRendering();
    //上报开始缓冲消息，等缓冲满了再进行播放
    onBufferStateChanged(0);
}

void GPlayer::pause() {
    onPlayStateChanged(STATE_PAUSED, 0);
    pauseThreads(true, true, true, true);
}

void GPlayer::resume() {
    onPlayStateChanged(STATE_STARTED, 0);
    resumeThreads(true, true, true, true);
}

void GPlayer::seekTo(uint32_t secondUs) {
    long second = secondUs;
    LOGI(TAG, "CoreFlow : seekTo %d", second);
    demuxerThread->setArgs(0, second);
}

void GPlayer::stop() {
    LOGI(TAG, "CoreFlow : stop playing");
    stopDemuxing();
    stopDecoding();
    stopRendering();
    stopMessageLoop();

    frameSource->flush();
    packetSource->flush();
    messageSource->flush();

    LOGI(TAG, "CoreFlow : player stopped %d %d %d %d %d", packetSource->audioSize(), packetSource->videoSize(),
            frameSource->audioSize(), frameSource->videoSize(), messageSource->size());
    onPlayStateChanged(STATE_STOPPED, 0);
}

int GPlayer::getDuration() {
    if (packetSource->getFormatInfo()) {
        return packetSource->getFormatInfo()->duration;
    }

    return 0;
}

int GPlayer::getVideoWidth() {
    if (packetSource->getVideoAVCodecParameters()) {
        return packetSource->getVideoAVCodecParameters()->width;
    }

    return 0;
}

int GPlayer::getVideoHeight() {
    if (packetSource->getVideoAVCodecParameters()) {
        return packetSource->getVideoAVCodecParameters()->height;
    }

    return 0;
}

void GPlayer::onDemuxingChanged(int state) {
    LOGI(TAG, "onDemuxingChanged %d", state);
    if (state == NOTIFY_END) {
        decoderHelper->setStopWhenEmpty(true);
    }
}

void GPlayer::onDecodingChanged(int state) {
    LOGI(TAG, "onDecodingChanged %d", state);
    if (state == NOTIFY_END) {
        renderHelper->setStopWhenEmpty(true);
    }
}

void GPlayer::onRenderingChanged(int state) {
    LOGI(TAG, "onRenderingChanged %d", state);
    if (state == NOTIFY_END) {
        if (isEof) {
            messageSource->pushMessage(MSG_DOMAIN_COMPLETE, 0, 0);
        }
    }
}

void GPlayer::onSeekStateChanged(int state) {
    playerLock.lock();
    if (seekState == state) {
        playerLock.unlock();
        return;
    }
    LOGI(TAG, "onSeekStateChanged %d-->%d", seekState, state);
    seekState = state;
    if (state == MSG_SEEK_START) {
        pauseThreads(true, true, true, false);
        packetSource->flush();
        frameSource->flush();
        packetSource->reset();
        frameSource->reset();
        messageSource->pushMessage(MSG_DOMAIN_BUFFER, 0, 0);
        decoderHelper->reset();
        resumeThreads(true, true, true, false);
    } else if (state == MSG_SEEK_END) {
    }
    messageHelper->notifyJava(MSG_DOMAIN_SEEK, seekState, 0, nullptr, nullptr);
    playerLock.unlock();
}

void GPlayer::onBufferStateChanged(int state) {
    playerLock.lock();
    if (state == bufferState || playState != STATE_STARTED) {
        playerLock.unlock();
        return;
    }
    LOGI(TAG, "onBufferStateChanged %d-->%d", bufferState, state);
    bufferState = state;
    messageHelper->notifyJava(MSG_DOMAIN_BUFFER, bufferState, bufferState, nullptr, nullptr);
    if (audioDecodeThread != nullptr && videoDecodeThread != nullptr) {
        if (state <= 0) {
            pauseThreads(false, true, false, false);
        } else {
            resumeThreads(false, true, false, false);
        }
    }
    playerLock.unlock();
}

void GPlayer::onPlayStateChanged(int state, long extra) {
    playerLock.lock();
    if (state == playState) {
        playerLock.unlock();
        return;
    }
    LOGI(TAG, "onPlayStateChanged %d-->%d", playState, state);
    playState = state;
    if (playState == STATE_PREPARED) {
        hasAudio = (extra & HAS_AUDIO) == HAS_AUDIO;
        hasVideo = (extra & HAS_VIDEO) == HAS_VIDEO;
        LOGI(TAG, "hasAudio %d, hasVideo %d", hasAudio, hasVideo);
    }
    messageHelper->notifyJava(MSG_DOMAIN_STATE, playState, 0, nullptr, nullptr);
    playerLock.unlock();
}

int GPlayer::processMessage(int arg1, long arg2) {
    Message *message;
    if (messageSource->readMessage(&message) > 0) {
        if (message->from == MSG_DOMAIN_ERROR) {
            messageHelper->handleErrorMessage(message);
        } else if (message->from == MSG_DOMAIN_STATE) {
            onPlayStateChanged(message->type, message->extra);
        } else if (message->from == MSG_DOMAIN_BUFFER) {
            onBufferStateChanged(message->type);
        } else if (message->from == MSG_DOMAIN_DEMUXING) {
            isEof = message->type == MSG_DEMUXING_EOF;
        } else if (message->from == MSG_DOMAIN_SEEK) {
            onSeekStateChanged(message->type);
        } else {
            messageHelper->notifyJava(message->from, message->type, message->extra, nullptr, nullptr);
        }
        messageSource->popMessage();
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return 0;
}

void GPlayer::startMessageLoop() {
    LOGI(TAG, "startMessageLoop");
    messageThread = LoopThreadHelper::createLoopThread(
            std::bind(&GPlayer::processMessage, this, std::placeholders::_1, std::placeholders::_2));
}

void GPlayer::stopMessageLoop() {
    if (messageThread && messageThread->hasStarted()) {
        messageThread->stop();
        messageSource->reset();
        messageThread->join();
        delete messageThread;
        messageThread = nullptr;
    }
    LOGI(TAG, "message thread was stopped!");
}

void GPlayer::startDemuxing(const std::string &url) {
    LOGI(TAG, "startDemuxing");
    demuxerHelper = new DemuxerHelper(url, packetSource, messageSource);
    demuxerThread = LoopThreadHelper::createLoopThread(
            std::bind(&DemuxerHelper::init, demuxerHelper),
            std::bind(&DemuxerHelper::readPacket, demuxerHelper, std::placeholders::_1, std::placeholders::_2),
            std::bind(&DemuxerHelper::release, demuxerHelper),
            std::bind(&GPlayer::onDemuxingChanged, this, std::placeholders::_1));
}

void GPlayer::stopDemuxing() {
    if (demuxerThread && demuxerThread->hasStarted()) {
        demuxerThread->stop();
        packetSource->reset();
        demuxerThread->join();
        delete demuxerThread;
        demuxerThread = nullptr;
    }
    delete demuxerHelper;
    demuxerHelper = nullptr;
    LOGI(TAG, "demuxing thread was stopped!");
}

void GPlayer::startDecoding() {
    LOGI(TAG, "startDecode");
    int mediaCodecFlag = (mFlags & AV_FLAG_SOURCE_MEDIA_CODEC) == AV_FLAG_SOURCE_MEDIA_CODEC;
    decoderHelper = new DecoderHelper(packetSource, frameSource, messageSource, hasAudio, hasVideo);
    decoderHelper->setMediaCodec(static_cast<bool>(mediaCodecFlag));
    decoderHelper->setANativeWindow(nativeWindow);
    if (decoderHelper->onInit() < 0) {
        messageHelper->notifyJava(MSG_DOMAIN_ERROR, 1, 0, "not support this codec", nullptr);
        return;
    }

    if (hasAudio) {
        audioDecodeThread = LoopThreadHelper::createLoopThread(
                std::bind(&DecoderHelper::processAudioBuffer, decoderHelper, std::placeholders::_1, std::placeholders::_2),
                std::bind(&GPlayer::onDecodingChanged, this, std::placeholders::_1));
    }

    if (hasVideo) {
        videoDecodeThread = LoopThreadHelper::createLoopThread(
                std::bind(&DecoderHelper::processVideoBuffer, decoderHelper, std::placeholders::_1, std::placeholders::_2),
                std::bind(&GPlayer::onDecodingChanged, this, std::placeholders::_1));
    }
}

void GPlayer::stopDecoding() {
    if (audioDecodeThread && audioDecodeThread->hasStarted()) {
        audioDecodeThread->stop();
        packetSource->reset();
        frameSource->reset();
        audioDecodeThread->join();
        delete audioDecodeThread;
        audioDecodeThread = nullptr;
    }
    if (videoDecodeThread && videoDecodeThread->hasStarted()) {
        videoDecodeThread->stop();
        packetSource->reset();
        frameSource->reset();
        videoDecodeThread->join();
        delete videoDecodeThread;
        videoDecodeThread = nullptr;
    }
    if (decoderHelper != nullptr) {
        decoderHelper->onRelease();
        delete decoderHelper;
        decoderHelper = nullptr;
    }
    LOGI(TAG, "decoding threads were stopped!");
}

void GPlayer::startRendering() {
    LOGI(TAG, "startRender");
    int mediaCodecFlag = (mFlags & AV_FLAG_SOURCE_MEDIA_CODEC) == AV_FLAG_SOURCE_MEDIA_CODEC;
    renderHelper = new RenderHelper(frameSource, messageSource, mediaCodecFlag, hasAudio, hasVideo);
    if (hasAudio) {
        int sampleRate = packetSource->getAudioAVCodecParameters()->sample_rate;
        int channels = packetSource->getAudioAVCodecParameters()->channels;
        int format = packetSource->getAudioAVCodecParameters()->format;
        int bytesPerSample = packetSource->getAudioAVCodecParameters()->frame_size;
        renderHelper->setAudioTrack(audioTrackJni);
        renderHelper->setAudioParams(sampleRate, channels, format, bytesPerSample);
        audioRenderThread = LoopThreadHelper::createLoopThread(
                std::bind(&RenderHelper::initAudioRenderer, renderHelper),
                std::bind(&RenderHelper::renderAudio, renderHelper, std::placeholders::_1, std::placeholders::_2),
                std::bind(&RenderHelper::releaseAudioRenderer, renderHelper),
                std::bind(&GPlayer::onRenderingChanged, this, std::placeholders::_1));
    }

    if (hasVideo) {
        int width = packetSource->getVideoAVCodecParameters()->width;
        int height = packetSource->getVideoAVCodecParameters()->height;
        renderHelper->setNativeWindow(nativeWindow);
        renderHelper->setVideoParams(width, height);
        videoRenderThread = LoopThreadHelper::createLoopThread(
                std::bind(&RenderHelper::initVideoRenderer, renderHelper),
                std::bind(&RenderHelper::renderVideo, renderHelper, std::placeholders::_1, std::placeholders::_2),
                std::bind(&RenderHelper::releaseVideoRenderer, renderHelper),
                std::bind(&GPlayer::onRenderingChanged, this, std::placeholders::_1));
    }
}

void GPlayer::stopRendering() {
    if (audioRenderThread && audioRenderThread->hasStarted()) {
        audioRenderThread->stop();
        frameSource->reset();
        audioRenderThread->join();
        delete audioRenderThread;
        audioRenderThread = nullptr;
    }
    if (videoRenderThread && videoRenderThread->hasStarted()) {
        videoRenderThread->stop();
        frameSource->reset();
        videoRenderThread->join();
        delete videoRenderThread;
        videoRenderThread = nullptr;
    }
    delete renderHelper;
    renderHelper = nullptr;
    LOGI(TAG, "rendering threads were stopped!");
}

void GPlayer::pauseThreads(bool demuxing, bool decoding, bool rendering, bool messaging) {
    if (demuxing) {
        demuxerThread->pause();
    }
    if (decoding) {
        if (hasAudio) audioDecodeThread->pause();
        if (hasVideo) videoDecodeThread->pause();
    }
    if (rendering) {
        if (hasAudio) audioRenderThread->pause();
        if (hasVideo) videoRenderThread->pause();
    }
    if (messaging) {
        messageThread->pause();
    }
}

void GPlayer::resumeThreads(bool demuxing, bool decoding, bool rendering, bool messaging) {
    if (demuxing) {
        demuxerThread->resume();
    }
    if (decoding) {
        if (hasAudio) audioDecodeThread->resume();
        if (hasVideo) videoDecodeThread->resume();
    }
    if (rendering) {
        if (hasAudio) audioRenderThread->resume();
        if (hasVideo) videoRenderThread->resume();
    }
    if (messaging) {
        messageThread->resume();
    }
}
