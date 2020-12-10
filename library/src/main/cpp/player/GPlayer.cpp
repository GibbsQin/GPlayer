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
    outputSource = new OutputSource();
    inputSource = new InputSource();
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

void GPlayer::av_init(FormatInfo *formatInfo) {
    inputSource->queueInfo(formatInfo);
    playerJni->onMessageCallback(MSG_TYPE_STATE, STATE_PREPARED, 0, nullptr, nullptr, nullptr);
}

uint32_t GPlayer::av_feed_audio(AVPacket *packet) {
    uint32_t result = inputSource->queueAudPkt(packet);
    playerJni->onMessageCallback(MSG_TYPE_SIZE, MSG_TYPE_SIZE_AUDIO_PACKET, result, nullptr,
                                 nullptr);
    return result;
}

uint32_t GPlayer::av_feed_video(AVPacket *packet) {
    uint32_t result = inputSource->queueVidPkt(packet);
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

    isDemuxing = true;
    demuxingThread = new DemuxingThread(std::bind(&GPlayer::startDemuxing, this,
                                                  std::placeholders::_1, std::placeholders::_2,
                                                  std::placeholders::_3, std::placeholders::_4),
                                        mUrl, mChannelId, MediaPipe::sFfmpegCallback, inputSource->getFormatInfo());
}

void GPlayer::start() {
    startDecode();
}

void GPlayer::pause() {

}

void GPlayer::seekTo(uint32_t secondMs) {

}

void GPlayer::stop() {
    isDemuxing = false;
    demuxingThread->join();
    LOGI(TAG, "CoreFlow : demuxing thread is stopped!");
    stopDecode();
    LOGI(TAG, "CoreFlow : decode threads were stopped!");
    codeInterceptor->onRelease();
    inputSource->flush();
    playerJni->onMessageCallback(MSG_TYPE_STATE, STATE_STOPPED, 0, nullptr, nullptr);
}

void GPlayer::startDecode() {
    LOGI(TAG, "CoreFlow : startDecode");
    int ret = codeInterceptor->onInit(inputSource->getFormatInfo());
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

    if (ret > 0) {
        playerJni->onMessageCallback(MSG_TYPE_ERROR, 1, 0,
                                     const_cast<char *>("not support this codec"), nullptr);
    }
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

void GPlayer::startDemuxing(char *web_url, int channelId, FfmpegCallback callback, FormatInfo *formatInfo) {
    ffmpeg_demuxing(web_url, channelId, callback, inputSource->getFormatInfo());
}

LoopFlag GPlayer::isDemuxingLoop() {
    if (isDemuxing) {
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

void GPlayer::onAudioThreadStart() {
    LOGI(TAG, "onAudioThreadStart");
}

int GPlayer::processAudioBuffer() {
    AVPacket *inPacket = nullptr;
    int ret = inputSource->dequeAudPkt(&inPacket);
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
    if (inputResult >= 0) {
        inputSource->popAudPkt();
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
    AVPacket *inPacket = nullptr;
    int ret = inputSource->dequeVidPkt(&inPacket);
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
    if (inputResult >= 0) {
        inputSource->popVidPkt();
    }

    return mediaSize;
}

void GPlayer::onVideoThreadEnd() {
    LOGI(TAG, "onVideoThreadEnd");
}

InputSource *GPlayer::getInputSource() {
    return inputSource;
}

OutputSource *GPlayer::getOutputSource() {
    return outputSource;
}
