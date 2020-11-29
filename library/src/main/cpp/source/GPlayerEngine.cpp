//
// Created by Gibbs on 2020/7/16.
//

#include "GPlayerEngine.h"
#include "MediaPipe.h"
#include <media/Media.h>
#include <base/Log.h>
#include <cstdint>
#include <unistd.h>
#include <interceptor/CodecInterceptor.h>

#define TAG "GPlayerImpC"

GPlayerEngine::GPlayerEngine(jobject jAVSource) {
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

GPlayerEngine::~GPlayerEngine() {
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

void GPlayerEngine::onInit() {
    LOGI(TAG, "CoreFlow : onInit");
    audioEngineThread = new CommonThread(ENGINE_FREQUENCY);
    audioEngineThread->setStartFunc(std::bind(&GPlayerEngine::onAudioThreadStart, this));
    audioEngineThread->setUpdateFunc(
            std::bind(&GPlayerEngine::processAudioBuffer, this, std::placeholders::_1));
    audioEngineThread->setEndFunc(std::bind(&GPlayerEngine::onAudioThreadEnd, this));
    audioEngineThread->start();

    videoEngineThread = new CommonThread(ENGINE_FREQUENCY);
    videoEngineThread->setStartFunc(std::bind(&GPlayerEngine::onVideoThreadStart, this));
    videoEngineThread->setUpdateFunc(
            std::bind(&GPlayerEngine::processVideoBuffer, this, std::placeholders::_1));
    videoEngineThread->setEndFunc(std::bind(&GPlayerEngine::onVideoThreadEnd, this));
    videoEngineThread->start();

    isRelease = false;
}

void GPlayerEngine::onRelease() {
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

MediaSource *GPlayerEngine::getInputSource() const {
    return inputSource;
}

MediaSourceJni *GPlayerEngine::getOutputSource() const {
    return outputSource;
}

void GPlayerEngine::onAudioThreadStart() {
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

void GPlayerEngine::processAudioBuffer(int64_t time) {
    if (isRelease || isStopping) {
        return;
    }
    MediaData *inPacket;
    int ret = inputSource->readAudioBuffer(&inPacket);
    if (ret <= 0) {
        if (ret == AV_SOURCE_RELEASE) {
            onRelease();
        }
        return;
    }
    outputSource->sendAudioPacketSize2Java(ret);
    int inputResult = -1;
    if (decodeFlag) {
        inputResult = codeInterceptor->inputBuffer(inPacket, AV_TYPE_AUDIO);
        MediaData *outBuffer;
        ret = codeInterceptor->outputBuffer(&outBuffer, AV_TYPE_AUDIO);
        if (ret >= 0) {
            mRemoteAudioQueueSize = outputSource->sendAudio2Java(outBuffer);
        }
    } else {
        mRemoteAudioQueueSize = outputSource->sendAudio2Java(inPacket);
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

void GPlayerEngine::onAudioThreadEnd() {
    LOGI(TAG, "stopAudioDecode");
    isAudioThreadRunning = false;
    onAllThreadEnd();
}

void GPlayerEngine::onVideoThreadStart() {
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

void GPlayerEngine::processVideoBuffer(int64_t time) {
    if (isRelease || isStopping) {
        return;
    }
    MediaData *inPacket;
    int ret = inputSource->readVideoBuffer(&inPacket);
    if (ret <= 0) {
        if (ret == AV_SOURCE_RELEASE) {
            onRelease();
        }
        return;
    }
    outputSource->sendVideoPacketSize2Java(ret);
    int inputResult = -1;
    if (decodeFlag) {
        inputResult = codeInterceptor->inputBuffer(inPacket, AV_TYPE_VIDEO);
        MediaData *outBuffer;
        ret = codeInterceptor->outputBuffer(&outBuffer, AV_TYPE_VIDEO);
        if (ret >= 0) {
            mRemoteVideoQueueSize = outputSource->sendVideo2Java(outBuffer);
        }
    } else {
        mRemoteVideoQueueSize = outputSource->sendVideo2Java(inPacket);
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

void GPlayerEngine::onVideoThreadEnd() {
    LOGI(TAG, "stopVideoDecode");
    isVideoThreadRunning = false;
    onAllThreadEnd();
}

void GPlayerEngine::onAllThreadEnd() {
    releaseMutex.lock();
    if (isVideoThreadRunning || isAudioThreadRunning) {
        releaseMutex.unlock();
        return;
    }
    if (isRelease) {
        releaseMutex.unlock();
        return;
    }
    isRelease = true;
    releaseMutex.unlock();
    LOGI(TAG, "onAllThreadEnd");
    if (decodeFlag) {
        codeInterceptor->onRelease();
    }
    outputSource->callJavaReleaseMethod();
}
