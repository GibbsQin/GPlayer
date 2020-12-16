//
// Created by Gibbs on 2020/7/16.
//

#include "GPlayer.h"
#include "MediaPipe.h"
#include "media/MediaData.h"
#include <base/Log.h>
#include <cstdint>
#include <unistd.h>
#include <demuxing/DemuxerHelper.h>

extern "C" {
#include <demuxing/avformat_def.h>
}

#define TAG "GPlayerC"

GPlayer::GPlayer(int channelId, uint32_t flag, jobject obj) {
    outputSource = new OutputSource();
    inputSource = new InputSource();
    playerJni = new GPlayerJni(obj);
    audioDecodeThread = nullptr;
    videoDecodeThread = nullptr;
    demuxingThread = nullptr;
    mFlags = flag;
    LOGI(TAG, "CoreFlow : start demuxing channel %d, flag %d", mChannelId, mFlags);
    mediaCodecFlag = (flag & AV_FLAG_SOURCE_MEDIA_CODEC) == AV_FLAG_SOURCE_MEDIA_CODEC;
    decodeHelper = new DecodeHelper(mediaCodecFlag);
    mChannelId = channelId;
    mIsPausing = false;
}

GPlayer::~GPlayer() {
    if (audioDecodeThread) {
        delete audioDecodeThread;
        audioDecodeThread = nullptr;
    }
    if (videoDecodeThread) {
        delete videoDecodeThread;
        videoDecodeThread = nullptr;
    }
    if (demuxingThread) {
        delete demuxingThread;
        demuxingThread = nullptr;
    }
    if (outputSource) {
        delete outputSource;
        outputSource = nullptr;
    }
    if (inputSource) {
        delete inputSource;
        inputSource = nullptr;
    }
    if (decodeHelper) {
        delete decodeHelper;
        decodeHelper = nullptr;
    }
    if (mUrl) {
        free(mUrl);
    }
    playerJni->onMessageCallback(MSG_TYPE_STATE, STATE_RELEASED, 0, nullptr, nullptr);
    if (playerJni) {
        delete playerJni;
        playerJni = nullptr;
    }
    LOGI(TAG, "CoreFlow : GPlayerImp destroyed");
}

void GPlayer::av_init(FormatInfo *formatInfo) {
    inputSource->queueInfo(formatInfo);
    playerJni->onMessageCallback(MSG_TYPE_STATE, STATE_PREPARED, 0, nullptr, nullptr, nullptr);
}

uint32_t GPlayer::av_feed_audio(AVPacket *packet, AVRational time_base) {
    uint32_t result = inputSource->queueAudPkt(packet, time_base);
    playerJni->onMessageCallback(MSG_TYPE_SIZE, MSG_TYPE_SIZE_AUDIO_PACKET, result, nullptr,
                                 nullptr);
    return result;
}

uint32_t GPlayer::av_feed_video(AVPacket *packet, AVRational time_base) {
    uint32_t result = inputSource->queueVidPkt(packet, time_base);
    playerJni->onMessageCallback(MSG_TYPE_SIZE, MSG_TYPE_SIZE_VIDEO_PACKET, result, nullptr,
                                 nullptr);
    return result;
}

void GPlayer::av_destroy() {

}

void GPlayer::av_error(int code, char *msg) {
    playerJni->onMessageCallback(MSG_TYPE_ERROR, code, 0, msg, nullptr);
}

void GPlayer::prepare(const std::string& url) {
    mUrl = static_cast<char *>(malloc(url.length() + 1));
    memcpy(mUrl, url.c_str(), url.length());
    mUrl[url.length()] = '\0';

    startDemuxing();
}

void GPlayer::start() {
    int ret = decodeHelper->onInit(inputSource->getFormatInfo());

    if (ret < 0) {
        playerJni->onMessageCallback(MSG_TYPE_ERROR, 1, 0,
                                     const_cast<char *>("not support this codec"), nullptr);
        return;
    }
    startDecode();
}

void GPlayer::pause() {
    mIsPausing = true;
    audioDecodeThread->pause();
    videoDecodeThread->pause();
}

void GPlayer::resume() {
    mIsPausing = false;
    audioDecodeThread->resume();
    videoDecodeThread->resume();
}

void GPlayer::seekTo(uint32_t secondUs) {
    mSeekUs = secondUs;
}

void GPlayer::stop() {
    isDemuxing = false;
    stopDemuxing();
    LOGI(TAG, "CoreFlow : demuxing thread is stopped!");
    stopDecode();
    LOGI(TAG, "CoreFlow : decode threads were stopped!");
    decodeHelper->onRelease();
    inputSource->flush();
    playerJni->onMessageCallback(MSG_TYPE_STATE, STATE_STOPPED, 0, nullptr, nullptr);
}

void GPlayer::setFlags(uint32_t flags) {
    mFlags = flags;
    mediaCodecFlag = (mFlags & AV_FLAG_SOURCE_MEDIA_CODEC) == AV_FLAG_SOURCE_MEDIA_CODEC;
    if (mediaCodecFlag) {
        decodeHelper->enableMediaCodec();
    }
}

void GPlayer::startDecode() {
    LOGI(TAG, "CoreFlow : startDecode");
    audioDecodeThread = new LoopThread();
    audioDecodeThread->setUpdateFunc(std::bind(&GPlayer::processAudioBuffer, this));
    audioDecodeThread->start();

    videoDecodeThread = new LoopThread();
    videoDecodeThread->setUpdateFunc(std::bind(&GPlayer::processVideoBuffer, this));
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
}

void GPlayer::startDemuxing() {
    auto demuxer = new DemuxerHelper(mUrl, this, inputSource->getFormatInfo());
    isDemuxing = true;
    demuxingThread = new LoopThread();
    demuxingThread->setStartFunc(std::bind(&DemuxerHelper::init, demuxer));
    demuxingThread->setUpdateFunc(std::bind(&DemuxerHelper::update, demuxer));
    demuxingThread->setEndFunc(std::bind(&DemuxerHelper::release, demuxer));
    demuxingThread->start();
}

void GPlayer::stopDemuxing() {
    if (demuxingThread && demuxingThread->hasStarted()) {
        demuxingThread->stop();
        demuxingThread->join();
    }
}

LoopFlag GPlayer::loopWait(int64_t *seekUs) {
    if (isDemuxing) {
        if (mSeekUs != -1) {
            *seekUs = mSeekUs;
            mSeekUs = -1;
        }
        uint32_t audioBufferSize = inputSource->getAudSize();
        uint32_t videoBufferSize = inputSource->getVidSize();
        if (audioBufferSize < MAX_BUFFER_SIZE && videoBufferSize < MAX_BUFFER_SIZE) {
            return LOOP;
        } else {
            return CONTINUE;
        }
    }
    return BREAK;
}

int GPlayer::processAudioBuffer() {
    AVPacket *inPacket = nullptr;
    int ret = inputSource->dequeAudPkt(&inPacket);
    if (ret <= 0) {
        return 0;
    }
    int mediaSize = 0;
    int inputResult = decodeHelper->inputBuffer(inPacket, AV_TYPE_AUDIO);
    MediaData *outBuffer = nullptr;
    ret = decodeHelper->outputBuffer(&outBuffer, AV_TYPE_AUDIO);
    if (ret >= 0) {
        mediaSize = outputSource->onReceiveAudio(outBuffer);
        playerJni->onMessageCallback(MSG_TYPE_SIZE, MSG_TYPE_SIZE_AUDIO_FRAME, mediaSize, nullptr,
                                     nullptr);
    }
    if (inputResult >= 0) {
        inputSource->popAudPkt(inPacket);
    }

    return mediaSize;
}

int GPlayer::processVideoBuffer() {
    AVPacket *inPacket = nullptr;
    int ret = inputSource->dequeVidPkt(&inPacket);
    if (ret <= 0) {
        return 0;
    }
    int mediaSize = 0;
    int inputResult = decodeHelper->inputBuffer(inPacket, AV_TYPE_VIDEO);
    MediaData *outBuffer = nullptr;
    ret = decodeHelper->outputBuffer(&outBuffer, AV_TYPE_VIDEO);
    if (ret >= 0) {
        mediaSize = outputSource->onReceiveVideo(outBuffer);
        playerJni->onMessageCallback(MSG_TYPE_SIZE, MSG_TYPE_SIZE_VIDEO_FRAME, mediaSize, nullptr,
                                     nullptr);
    }
    if (inputResult >= 0) {
        inputSource->popVidPkt(inPacket);
    }

    return mediaSize;
}

InputSource *GPlayer::getInputSource() {
    return inputSource;
}

OutputSource *GPlayer::getOutputSource() {
    return outputSource;
}
