//
// Created by Gibbs on 2020/7/16.
//

#include "GPlayerEngine.h"
#include "MediaPipe.h"
#include "DemuxingThread.h"
#include <media/Media.h>
#include <base/Log.h>
#include <cstdint>
#include <unistd.h>
#include <interceptor/CodecInterceptor.h>

#define TAG "GPlayerImpC"

GPlayerEngine::GPlayerEngine(jobject jAVSource) {
    outputSource = new MediaSourceJni(jAVSource);
    std::string url = outputSource->getUrl();
    inputSource = new MediaSource(url.c_str(), 0);
    audioEngineThread = nullptr;
    videoEngineThread = nullptr;
    decodeFlag = (outputSource->getFlag() & AV_FLAG_SOURCE_DECODE) == AV_FLAG_SOURCE_DECODE;
    mediaCodecFlag =
            (outputSource->getFlag() & AV_FLAG_SOURCE_MEDIA_CODEC) == AV_FLAG_SOURCE_MEDIA_CODEC;
    LOGI(TAG, "CoreFlow : new GPlayerImp decodeFlag = %d, mediaCodecFlag = %d", decodeFlag,
         mediaCodecFlag);
    isRelease = false;
    isStopping = false;
    if (decodeFlag) {
        codeInterceptor = new CodecInterceptor(mediaCodecFlag);
    }

    mUrl = static_cast<char *>(malloc(url.length()));
    memcpy(mUrl, url.c_str(), url.length());
    LOGI(TAG, "CoreFlow : start demuxing url %s, channel %d", mUrl, inputSource->getChannelId());
    isDemuxing = true;
    demuxingThread = new DemuxingThread(std::bind(&GPlayerEngine::startDemuxing, this,
                                 std::placeholders::_1, std::placeholders::_2,
                                 std::placeholders::_3, std::placeholders::_4),
                       mUrl, inputSource->getChannelId(),
                       MediaPipe::sFfmpegCallback, inputSource->getAVHeader());
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
    if (mUrl) {
        free(mUrl);
    }
    LOGE(TAG, "CoreFlow : GPlayerImp destroyed");
}

void GPlayerEngine::onInit() {
    LOGI(TAG, "CoreFlow : onInit");
    audioEngineThread = new CommonThread();
    audioEngineThread->setStartFunc(std::bind(&GPlayerEngine::onAudioThreadStart, this));
    audioEngineThread->setUpdateFunc(std::bind(&GPlayerEngine::processAudioBuffer, this));
    audioEngineThread->setEndFunc(std::bind(&GPlayerEngine::onAudioThreadEnd, this));
    audioEngineThread->start();

    videoEngineThread = new CommonThread();
    videoEngineThread->setStartFunc(std::bind(&GPlayerEngine::onVideoThreadStart, this));
    videoEngineThread->setUpdateFunc(std::bind(&GPlayerEngine::processVideoBuffer, this));
    videoEngineThread->setEndFunc(std::bind(&GPlayerEngine::onVideoThreadEnd, this));
    videoEngineThread->start();

    MediaInfo *header = inputSource->getAVHeader();
    if (decodeFlag) {
        int ret = codeInterceptor->onInit(header);
        if (ret > 0) {
            outputSource->callJavaErrorMethod(ret, "not support this codec");
        }
    }
    outputSource->callJavaInitMethod(inputSource->getAVHeader(), inputSource->getChannelId());

    isRelease = false;
}

void GPlayerEngine::onRelease() {
    if (isStopping || isRelease) {
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
    audioEngineThread->join();
    videoEngineThread->join();
    LOGI(TAG, "onAllThreadEnd");
    if (decodeFlag) {
        codeInterceptor->onRelease();
    }
    outputSource->callJavaReleaseMethod();
    isRelease = true;
}

MediaSource *GPlayerEngine::getInputSource() const {
    return inputSource;
}

MediaSourceJni *GPlayerEngine::getOutputSource() const {
    return outputSource;
}

void GPlayerEngine::startDemuxing(char *web_url, int channelId, FfmpegCallback callback,
                                  MediaInfo *mediaInfo) {
    ffmpeg_demuxing(web_url, channelId, callback, mediaInfo);
}

void GPlayerEngine::stopDemuxingLoop() {
    isDemuxing = false;
//    demuxingThread->join();
//    LOGI(TAG, "CoreFlow : demuxing thread is stopped!");
}

void GPlayerEngine::onAudioThreadStart() {
    LOGI(TAG, "startAudioDecode");
}

int GPlayerEngine::processAudioBuffer() {
    if (isRelease || isStopping) {
        return 0;
    }
    MediaData *inPacket;
    int ret = inputSource->readAudioBuffer(&inPacket);
    if (ret <= 0) {
        if (ret == AV_SOURCE_RELEASE) {
            onRelease();
        }
        return 0;
    }
    outputSource->sendAudioPacketSize2Java(ret);
    int inputResult = -1;
    int mediaSize = 0;
    if (decodeFlag) {
        inputResult = codeInterceptor->inputBuffer(inPacket, AV_TYPE_AUDIO);
        MediaData *outBuffer;
        ret = codeInterceptor->outputBuffer(&outBuffer, AV_TYPE_AUDIO);
        if (ret >= 0) {
            mediaSize = outputSource->sendAudio2Java(outBuffer);
        }
    } else {
        mediaSize = outputSource->sendAudio2Java(inPacket);
    }
    if (inputResult != TRY_AGAIN) {
        free(inPacket->data);
        delete inPacket;
        inputSource->popAudioBuffer();
    }

    return mediaSize;
}

void GPlayerEngine::onAudioThreadEnd() {
    LOGI(TAG, "stopAudioDecode");
}

void GPlayerEngine::onVideoThreadStart() {
    LOGI(TAG, "startVideoDecode");
}

int GPlayerEngine::processVideoBuffer() {
    if (isRelease || isStopping) {
        return 0;
    }
    MediaData *inPacket;
    int ret = inputSource->readVideoBuffer(&inPacket);
    if (ret <= 0) {
        if (ret == AV_SOURCE_RELEASE) {
            onRelease();
        }
        return 0;
    }
    outputSource->sendVideoPacketSize2Java(ret);
    int inputResult = -1;
    int mediaSize = 0;
    if (decodeFlag) {
        inputResult = codeInterceptor->inputBuffer(inPacket, AV_TYPE_VIDEO);
        MediaData *outBuffer;
        ret = codeInterceptor->outputBuffer(&outBuffer, AV_TYPE_VIDEO);
        if (ret >= 0) {
            mediaSize = outputSource->sendVideo2Java(outBuffer);
        }
    } else {
        mediaSize = outputSource->sendVideo2Java(inPacket);
    }
    if (inputResult != TRY_AGAIN) {
        free(inPacket->data);
        delete inPacket;
        inputSource->popVideoBuffer();
    }

    return mediaSize;
}

void GPlayerEngine::onVideoThreadEnd() {
    LOGI(TAG, "stopVideoDecode");
}
