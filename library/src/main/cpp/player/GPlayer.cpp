//
// Created by Gibbs on 2020/7/16.
//

#include "GPlayer.h"
#include "MediaPipe.h"
#include "DemuxingThread.h"
#include "media/MediaData.h"
#include <base/Log.h>
#include <cstdint>
#include <unistd.h>
#include <interceptor/CodecInterceptor.h>
#include <media/MediaHelper.h>

extern "C" {
#include <demuxing/avformat_def.h>
}

#define TAG "GPlayerC"

GPlayer::GPlayer(int channelId, uint32_t flag, jobject obj) {
    outputSource = new MediaSource();
    inputSource = new MediaSource();
    playerJni = new GPlayerJni(obj);
    audioEngineThread = nullptr;
    videoEngineThread = nullptr;
    LOGI(TAG, "CoreFlow : start demuxing channel %d, flag %d", mChannelId, flag);
    mediaCodecFlag = (flag & AV_FLAG_SOURCE_MEDIA_CODEC) == AV_FLAG_SOURCE_MEDIA_CODEC;
    codeInterceptor = new CodecInterceptor(mediaCodecFlag);
    mChannelId = channelId;
}

GPlayer::~GPlayer() {
    if (audioEngineThread) {
        delete audioEngineThread;
        audioEngineThread = nullptr;
    }
    if (videoEngineThread) {
        delete videoEngineThread;
        videoEngineThread = nullptr;
    }
    if (outputSource) {
        delete outputSource;
        outputSource = nullptr;
    }
    if (inputSource) {
        delete inputSource;
        inputSource = nullptr;
    }
    if (codeInterceptor) {
        delete codeInterceptor;
        codeInterceptor = nullptr;
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

void GPlayer::av_init(FormatInfo formatInfo) {
    inputSource->onInit(formatInfo);
    playerJni->onMessageCallback(MSG_TYPE_STATE, STATE_PREPARED, 0, nullptr, nullptr, nullptr);
}

uint32_t GPlayer::av_feed_audio(uint8_t *pInputBuf, uint32_t dwInputDataSize,
                                uint64_t u64InputPTS, uint64_t u64InputDTS, int flag) {
    auto *avData = new MediaData(pInputBuf, dwInputDataSize, nullptr, 0, nullptr, 0);
    avData->pts = u64InputPTS;
    avData->dts = u64InputDTS;
    avData->flag = static_cast<uint8_t>(flag);

    uint32_t result = inputSource->onReceiveAudio(avData);
    playerJni->onMessageCallback(MSG_TYPE_SIZE, MSG_TYPE_SIZE_AUDIO_PACKET, result, nullptr,
                                 nullptr);
    return result;
}

uint32_t GPlayer::av_feed_video(uint8_t *pInputBuf, uint32_t dwInputDataSize,
                                uint64_t u64InputPTS, uint64_t u64InputDTS, int flag) {
    auto *avData = new MediaData(pInputBuf, dwInputDataSize, nullptr, 0, nullptr, 0);
    avData->pts = u64InputPTS;
    avData->dts = u64InputDTS;
    avData->flag = static_cast<uint8_t>(flag);

    uint32_t result = inputSource->onReceiveVideo(avData);
    playerJni->onMessageCallback(MSG_TYPE_SIZE, MSG_TYPE_SIZE_VIDEO_PACKET, result, nullptr,
                                 nullptr);
    return result;
}

void GPlayer::av_destroy() {
    inputSource->onRelease();
}

void GPlayer::av_error(int code, char *msg) {
    playerJni->onMessageCallback(MSG_TYPE_ERROR, code, 0, msg, nullptr);
}

void GPlayer::prepare(const std::string& url) {
    mUrl = static_cast<char *>(malloc(url.length() + 1));
    memcpy(mUrl, url.c_str(), url.length());
    mUrl[url.length()] = '\0';

    isDemuxing = true;
    demuxingThread = new DemuxingThread(std::bind(&GPlayer::startDemuxing, this,
                                                  std::placeholders::_1, std::placeholders::_2,
                                                  std::placeholders::_3),
                                        mUrl, mChannelId,
                                        MediaPipe::sFfmpegCallback);
}

void GPlayer::start() {
    startDecode();
}

void GPlayer::pause() {

}

void GPlayer::seekTo(uint32_t secondMs) {

}

void GPlayer::stop() {
    LOGI(TAG, "CoreFlow : stopping");
    isDemuxing = false;
    demuxingThread->join();
    LOGI(TAG, "CoreFlow : demuxing thread is stopped!");
    stopDecode();
    LOGI(TAG, "CoreFlow : decode threads were stopped!");
    codeInterceptor->onRelease();
    inputSource->flushBuffer();
    playerJni->onMessageCallback(MSG_TYPE_STATE, STATE_STOPPED, 0, nullptr, nullptr);
}

void GPlayer::startDecode() {
    LOGI(TAG, "CoreFlow : startDecode");
    audioEngineThread = new DecodeThread();
    audioEngineThread->setStartFunc(std::bind(&GPlayer::onAudioThreadStart, this));
    audioEngineThread->setUpdateFunc(std::bind(&GPlayer::processAudioBuffer, this));
    audioEngineThread->setEndFunc(std::bind(&GPlayer::onAudioThreadEnd, this));
    audioEngineThread->start();

    videoEngineThread = new DecodeThread();
    videoEngineThread->setStartFunc(std::bind(&GPlayer::onVideoThreadStart, this));
    videoEngineThread->setUpdateFunc(std::bind(&GPlayer::processVideoBuffer, this));
    videoEngineThread->setEndFunc(std::bind(&GPlayer::onVideoThreadEnd, this));
    videoEngineThread->start();

    int ret = codeInterceptor->onInit(inputSource->getFormatInfo());
    if (ret > 0) {
        playerJni->onMessageCallback(MSG_TYPE_ERROR, 1, 0,
                                     const_cast<char *>("not support this codec"), nullptr);
    }
    outputSource->onInit(inputSource->getFormatInfo());
}

void GPlayer::stopDecode() {
    if (audioEngineThread && audioEngineThread->hasStarted()) {
        audioEngineThread->stop();
        audioEngineThread->join();
    }
    if (videoEngineThread && videoEngineThread->hasStarted()) {
        videoEngineThread->stop();
        videoEngineThread->join();
    }
}

void GPlayer::startDemuxing(char *web_url, int channelId, FfmpegCallback callback) {
    ffmpeg_demuxing(web_url, channelId, callback);
}

LoopFlag GPlayer::isDemuxingLoop() {
    if (isDemuxing) {
        uint32_t audioBufferSize = inputSource->getAudioBufferSize();
        uint32_t videoBufferSize = inputSource->getVideoBufferSize();
        if (audioBufferSize < MAX_BUFFER_SIZE && videoBufferSize < MAX_BUFFER_SIZE) {
            return LOOP;
        } else {
            return CONTINUE;
        }
    }
    return BREAK;
}

void GPlayer::onAudioThreadStart() {
    LOGI(TAG, "onAudioThreadStart");
}

int GPlayer::processAudioBuffer() {
    MediaData *inPacket = nullptr;
    int ret = inputSource->readAudioBuffer(&inPacket);
    if (ret <= 0) {
        return 0;
    }
    int mediaSize = 0;
    int inputResult = codeInterceptor->inputBuffer(inPacket, AV_TYPE_AUDIO);
    MediaData *outBuffer = nullptr;
    ret = codeInterceptor->outputBuffer(&outBuffer, AV_TYPE_AUDIO);
    if (ret >= 0) {
        auto desBuffer = new MediaData();
        MediaHelper::copy(outBuffer, desBuffer);
        mediaSize = outputSource->onReceiveAudio(desBuffer);
        playerJni->onMessageCallback(MSG_TYPE_SIZE, MSG_TYPE_SIZE_AUDIO_FRAME, mediaSize, nullptr,
                                     nullptr);
    }
    if (inputResult != TRY_AGAIN) {
        inputSource->popAudioBuffer();
    }

    return mediaSize;
}

void GPlayer::onAudioThreadEnd() {
    LOGI(TAG, "onAudioThreadEnd");
}

void GPlayer::onVideoThreadStart() {
    LOGI(TAG, "onVideoThreadStart");
}

int GPlayer::processVideoBuffer() {
    MediaData *inPacket = nullptr;
    int ret = inputSource->readVideoBuffer(&inPacket);
    if (ret <= 0) {
        return 0;
    }
    int mediaSize = 0;
    int inputResult = codeInterceptor->inputBuffer(inPacket, AV_TYPE_VIDEO);
    MediaData *outBuffer = nullptr;
    ret = codeInterceptor->outputBuffer(&outBuffer, AV_TYPE_VIDEO);
    if (ret >= 0) {
        auto desBuffer = new MediaData();
        MediaHelper::copy(outBuffer, desBuffer);
        mediaSize = outputSource->onReceiveVideo(desBuffer);
        playerJni->onMessageCallback(MSG_TYPE_SIZE, MSG_TYPE_SIZE_VIDEO_FRAME, mediaSize, nullptr,
                                     nullptr);
    }
    if (inputResult != TRY_AGAIN) {
        inputSource->popVideoBuffer();
    }

    return mediaSize;
}

void GPlayer::onVideoThreadEnd() {
    LOGI(TAG, "onVideoThreadEnd");
}

MediaSource *GPlayer::getFrameSource() {
    return outputSource;
}
