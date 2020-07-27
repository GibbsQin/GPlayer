//
// Created by Gibbs on 2020/7/16.
//

#include "GPlayerImp.h"
#include "GPlayerMgr.h"
#include <media/Media.h>
#include <base/Log.h>
#include <cstdint>
#include <unistd.h>
#include <interceptor/CodecInterceptor.h>

#define TAG "GPlayerImp"

GPlayerImp::GPlayerImp(jobject jAVSource) {
    outputSource = new MediaSourceJni(jAVSource);
    inputSource = new MediaSource();
    audioEngineThread = nullptr;
    videoEngineThread = nullptr;
    mRemoteAudioQueueSize = 0;
    mRemoteVideoQueueSize = 0;
    mAudioSleepTimeUs = 0;
    mVideoSleepTimeUs = 0;
    decodeFlag = (outputSource->getFlag() & AV_FLAG_SOURCE_DECODE) == AV_FLAG_SOURCE_DECODE;
    mediaCodecFlag = (outputSource->getFlag() & AV_FLAG_SOURCE_MEDIA_CODEC) == AV_FLAG_SOURCE_MEDIA_CODEC;
    LOGI(TAG, "CoreFlow : new GPlayerImp decodeFlag = %d, mediaCodecFlag = %d", decodeFlag, mediaCodecFlag);
    hasInit = false;
    isRelease = false;
    isStopping = false;
    if (decodeFlag) {
        codeInterceptor = new CodecInterceptor(mediaCodecFlag);
    }
}

GPlayerImp::~GPlayerImp() {
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
    if (decodeFlag) {
        if (codeInterceptor) {
            delete codeInterceptor;
            codeInterceptor = nullptr;
        }
    }
    LOGE(TAG, "CoreFlow : GPlayerImp destroyed");
}

void GPlayerImp::onInit() {
    LOGI(TAG, "CoreFlow : onInit");
    audioEngineThread = new CommonThread(ENGINE_FREQUENCY);
    audioEngineThread->setStartFunc(std::bind(&GPlayerImp::onAudioThreadStart, this));
    audioEngineThread->setUpdateFunc(
            std::bind(&GPlayerImp::processAudioBuffer, this, std::placeholders::_1));
    audioEngineThread->setEndFunc(std::bind(&GPlayerImp::onAudioThreadEnd, this));
    audioEngineThread->start();

    videoEngineThread = new CommonThread(ENGINE_FREQUENCY);
    videoEngineThread->setStartFunc(std::bind(&GPlayerImp::onVideoThreadStart, this));
    videoEngineThread->setUpdateFunc(
            std::bind(&GPlayerImp::processVideoBuffer, this, std::placeholders::_1));
    videoEngineThread->setEndFunc(std::bind(&GPlayerImp::onVideoThreadEnd, this));
    videoEngineThread->start();

    isRelease = false;
}

void GPlayerImp::onRelease() {
    releaseMutex.lock();
    if (isStopping) {
        return;
    }
    LOGE(TAG, "CoreFlow : onRelease");
    isStopping = true;
    if (audioEngineThread && audioEngineThread->hasStarted()) {
        audioEngineThread->stop();
    }
    if (videoEngineThread && videoEngineThread->hasStarted()) {
        videoEngineThread->stop();
    }
    releaseMutex.unlock();
}

MediaSource *GPlayerImp::getInputSource() const {
    return inputSource;
}

MediaSourceJni *GPlayerImp::getOutputSource() const {
    return outputSource;
}

void GPlayerImp::onAudioThreadStart() {
    LOGI(TAG, "startAudioDecode");
    isAudioThreadRunning = true;
    MediaInfo *header = inputSource->getAVHeader();
    if (decodeFlag) {
        int ret = codeInterceptor->onInit(header);
        if (ret > 0) {
            outputSource->callJavaErrorMethod(ret, "not support this codec");
        }
    }
    if (isVideoThreadRunning && !hasInit) {
        hasInit = true;
        outputSource->callJavaInitMethod(inputSource->getAVHeader(), inputSource->getChannelId());
    }
}

void GPlayerImp::processAudioBuffer(int64_t time) {
    if (isRelease || isStopping) {
        return;
    }
    MediaData *inPacket;
    int ret = inputSource->readAudioBuffer(&inPacket);
    if (ret == AV_SOURCE_RELEASE) {
        onRelease();
        return;
    }
    if (ret <= 0) {
        return;
    }
    outputSource->sendAudioPacketSize2Java(ret);
    MediaData *outBuffer = inPacket;
    int inputResult = -1;
    if (decodeFlag) {
        inputResult = codeInterceptor->inputBuffer(inPacket, AV_TYPE_AUDIO);
        ret = codeInterceptor->outputBuffer(&outBuffer, AV_TYPE_AUDIO);
        if (ret >= 0) {
            mRemoteAudioQueueSize = outputSource->sendAudio2Java(outBuffer);
        }
    } else {
        mRemoteAudioQueueSize = outputSource->sendAudio2Java(outBuffer);
    }
    if (inputResult != TRY_AGAIN) {
        free(inPacket->data);
        delete inPacket;
        inputSource->popAudioBuffer();
    }

    if (mRemoteAudioQueueSize > MAX_OUTPUT_FRAME_SIZE) {
        mAudioSleepTimeUs += SLEEP_TIME_GAP;
    } else {
        mAudioSleepTimeUs = 0;
    }
    if (mAudioSleepTimeUs > 0) {
        usleep(static_cast<useconds_t>(mAudioSleepTimeUs));
    }
}

void GPlayerImp::onAudioThreadEnd() {
    LOGI(TAG, "stopAudioDecode");
    isAudioThreadRunning = false;
    onAllThreadEnd();
}

void GPlayerImp::onVideoThreadStart() {
    MediaInfo *header = inputSource->getAVHeader();
    LOGI(TAG, "startVideoDecode");
    isVideoThreadRunning = true;
    if (decodeFlag) {
        int ret = codeInterceptor->onInit(header);
        if (ret > 0) {
            outputSource->callJavaErrorMethod(ret, "not support this codec");
        }
    }
    if (isAudioThreadRunning && !hasInit) {
        hasInit = true;
        outputSource->callJavaInitMethod(inputSource->getAVHeader(), inputSource->getChannelId());
    }
}

void GPlayerImp::processVideoBuffer(int64_t time) {
    if (isRelease || isStopping) {
        return;
    }
    MediaData *inPacket;
    int ret = inputSource->readVideoBuffer(&inPacket);
    if (ret == AV_SOURCE_RELEASE) {
        onRelease();
        return;
    }
    if (ret <= 0) {
        return;
    }
    outputSource->sendVideoPacketSize2Java(ret);
    MediaData *outBuffer = inPacket;
    int inputResult = -1;
    if (decodeFlag) {
        inputResult = codeInterceptor->inputBuffer(inPacket, AV_TYPE_VIDEO);
        ret = codeInterceptor->outputBuffer(&outBuffer, AV_TYPE_VIDEO);
        if (ret >= 0) {
            mRemoteVideoQueueSize = outputSource->sendVideo2Java(outBuffer);
        }
    } else {
        mRemoteVideoQueueSize = outputSource->sendVideo2Java(outBuffer);
    }
    if (inputResult != TRY_AGAIN) {
        free(inPacket->data);
        delete inPacket;
        inputSource->popVideoBuffer();
    }

    if (mRemoteVideoQueueSize > MAX_OUTPUT_FRAME_SIZE) {
        mVideoSleepTimeUs += SLEEP_TIME_GAP;
    } else {
        mVideoSleepTimeUs = 0;
    }
    if (mVideoSleepTimeUs > 0) {
        usleep(static_cast<useconds_t>(mVideoSleepTimeUs));
    }
}

void GPlayerImp::onVideoThreadEnd() {
    LOGI(TAG, "stopVideoDecode");
    isVideoThreadRunning = false;
    onAllThreadEnd();
}

void GPlayerImp::onAllThreadEnd() {
    if (isVideoThreadRunning || isAudioThreadRunning) {
        return;
    }
    if (isRelease) {
        return;
    }
    isRelease = true;
    LOGI(TAG, "onAllThreadEnd");
    outputSource->callJavaReleaseMethod();
    if (decodeFlag) {
        codeInterceptor->onRelease();
    }
}
