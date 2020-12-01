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

GPlayerEngine::GPlayerEngine(int channelId, jobject jAVSource) {
    outputSource = new MediaSourceJni(jAVSource);
    inputSource = new MediaSource();
    audioEngineThread = nullptr;
    videoEngineThread = nullptr;
    mediaCodecFlag =
            (outputSource->getFlag() & AV_FLAG_SOURCE_MEDIA_CODEC) == AV_FLAG_SOURCE_MEDIA_CODEC;
    LOGI(TAG, "CoreFlow : new GPlayerImp mediaCodecFlag = %d", mediaCodecFlag);
    codeInterceptor = new CodecInterceptor(mediaCodecFlag);

    std::string url = outputSource->getUrl();
    mUrl = static_cast<char *>(malloc(url.length()));
    memcpy(mUrl, url.c_str(), url.length());
    mChannelId = channelId;
    LOGI(TAG, "CoreFlow : start demuxing url %s, channel %d", mUrl, mChannelId);
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
    if (codeInterceptor) {
        delete codeInterceptor;
        codeInterceptor = nullptr;
    }
    if (mUrl) {
        free(mUrl);
    }
    LOGE(TAG, "CoreFlow : GPlayerImp destroyed");
}

void GPlayerEngine::av_init(MediaInfo *header) {
    inputSource->onInit(header);
    startDecode();
}

uint32_t GPlayerEngine::av_feed_audio(uint8_t *pInputBuf, uint32_t dwInputDataSize,
                                      uint64_t u64InputPTS, uint64_t u64InputDTS, int flag) {
    auto *avData = new MediaData();
    avData->data = static_cast<uint8_t *>(malloc(dwInputDataSize));
    memcpy(avData->data, pInputBuf, dwInputDataSize);
    avData->size = dwInputDataSize;
    avData->pts = u64InputPTS;
    avData->dts = u64InputDTS;
    avData->flag = flag;

    return inputSource->onReceiveAudio(avData);
}

uint32_t GPlayerEngine::av_feed_video(uint8_t *pInputBuf, uint32_t dwInputDataSize,
                                      uint64_t u64InputPTS, uint64_t u64InputDTS, int flag) {
    auto *avData = new MediaData();
    avData->data = static_cast<uint8_t *>(malloc(dwInputDataSize));
    memcpy(avData->data, pInputBuf, dwInputDataSize);
    avData->size = dwInputDataSize;
    avData->pts = u64InputPTS;
    avData->dts = u64InputDTS;
    avData->flag = flag;

    return inputSource->onReceiveVideo(avData);
}

void GPlayerEngine::av_destroy() {
    inputSource->onRelease();
}

void GPlayerEngine::av_error(int code, char *msg) {
    outputSource->callJavaErrorMethod(code, msg);
}

void GPlayerEngine::start() {
    isDemuxing = true;
    demuxingThread = new DemuxingThread(std::bind(&GPlayerEngine::startDemuxing, this,
                                                  std::placeholders::_1, std::placeholders::_2,
                                                  std::placeholders::_3, std::placeholders::_4),
                                        mUrl, mChannelId,
                                        MediaPipe::sFfmpegCallback, inputSource->getAVHeader());
}

void GPlayerEngine::stop() {
    LOGI(TAG, "CoreFlow : stopping");
    isDemuxing = false;
    demuxingThread->join();
    LOGI(TAG, "CoreFlow : demuxing thread is stopped!");
    stopDecode();;
    LOGI(TAG, "CoreFlow : decode threads were stopped!");
    codeInterceptor->onRelease();
    inputSource->flushBuffer();
    outputSource->callJavaReleaseMethod();
}

void GPlayerEngine::startDecode() {
    LOGI(TAG, "CoreFlow : startDecode");
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
    int ret = codeInterceptor->onInit(header);
    if (ret > 0) {
        outputSource->callJavaErrorMethod(ret, "not support this codec");
    }
    outputSource->callJavaInitMethod(inputSource->getAVHeader());
}

void GPlayerEngine::stopDecode() {
    if (audioEngineThread && audioEngineThread->hasStarted()) {
        audioEngineThread->stop();
    }
    if (videoEngineThread && videoEngineThread->hasStarted()) {
        videoEngineThread->stop();
    }
    audioEngineThread->join();
    videoEngineThread->join();
}

void GPlayerEngine::startDemuxing(char *web_url, int channelId, FfmpegCallback callback,
                                  MediaInfo *mediaInfo) {
    ffmpeg_demuxing(web_url, channelId, callback, mediaInfo);
}

void GPlayerEngine::onAudioThreadStart() {
    LOGI(TAG, "startAudioDecode");
}

int GPlayerEngine::processAudioBuffer() {
    MediaData *inPacket;
    int ret = inputSource->readAudioBuffer(&inPacket);
    if (ret <= 0) {
        return 0;
    }
    outputSource->sendAudioPacketSize2Java(ret);
    int inputResult = -1;
    int mediaSize = 0;
    inputResult = codeInterceptor->inputBuffer(inPacket, AV_TYPE_AUDIO);
    MediaData *outBuffer;
    ret = codeInterceptor->outputBuffer(&outBuffer, AV_TYPE_AUDIO);
    if (ret >= 0) {
        mediaSize = outputSource->sendAudio2Java(outBuffer);
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
    MediaData *inPacket;
    int ret = inputSource->readVideoBuffer(&inPacket);
    if (ret <= 0) {
        return 0;
    }
    outputSource->sendVideoPacketSize2Java(ret);
    int inputResult = -1;
    int mediaSize = 0;
    inputResult = codeInterceptor->inputBuffer(inPacket, AV_TYPE_VIDEO);
    MediaData *outBuffer;
    ret = codeInterceptor->outputBuffer(&outBuffer, AV_TYPE_VIDEO);
    if (ret >= 0) {
        mediaSize = outputSource->sendVideo2Java(outBuffer);
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
