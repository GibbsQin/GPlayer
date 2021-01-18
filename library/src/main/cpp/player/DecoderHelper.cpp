/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#include <base/Log.h>
#include <codec/FfmpegAudioDecoder.h>
#include <codec/FfmpegVideoDecoder.h>
#include <codec/MediaCodecVideoDecoder.h>
#include <codec/MediaCodecAudioDecoder.h>
#include <thread>
#include <base/LoopThread.h>
#include "DecoderHelper.h"

extern "C" {
#include <demuxing/avformat_def.h>
}

#define TAG "DecoderHelper"

DecoderHelper::DecoderHelper(PacketSource *inputSource, FrameSource *outputSource, MessageSource *messageSource,
                             bool hasAudio, bool hasVideo) {
    this->inputSource = inputSource;
    this->outputSource = outputSource;
    this->messageSource = messageSource;
    this->hasAudio = hasAudio;
    this->hasVideo = hasVideo;
    mediaCodecFirst = false;
    hasInit = false;
}

DecoderHelper::~DecoderHelper() = default;

int DecoderHelper::onInit() {
    audioLock.lock();
    videoLock.lock();
    if (hasInit) {
        videoLock.unlock();
        audioLock.unlock();
        return -1;
    }
    hasInit = true;
    if (hasAudio) {
        createAudioDecoder();
    }
    if (hasVideo) {
        createVideoDecoder();
    }
    videoLock.unlock();
    audioLock.unlock();
    return 0;
}

void DecoderHelper::createAudioDecoder() {
    FormatInfo *formatInfo = inputSource->getFormatInfo();
    AVCodecParameters *audioParameters = avcodec_parameters_alloc();
    avcodec_parameters_copy(audioParameters, formatInfo->audcodecpar);
    bool mediaCodecSupport = static_cast<bool>(get_mime_by_codec_id((CODEC_TYPE) audioParameters->codec_id));
    if (/*mediaCodecFirst*/false) {
        if (mediaCodecSupport) {
            audioDecoder = new MediaCodecAudioDecoder();
        } else {
            audioDecoder = new FfmpegAudioDecoder();
        }
    } else {
        audioDecoder = new FfmpegAudioDecoder();
    }
    auto audioDataSize =
            audioParameters->frame_size * audioParameters->format * audioParameters->channels;
    audioOutFrame = new MediaData(nullptr, static_cast<uint32_t>(audioDataSize), nullptr, 0,
                                  nullptr, 0);
    audioDecoder->init(audioParameters);
    avcodec_parameters_free(&audioParameters);
}

void DecoderHelper::createVideoDecoder() {
    FormatInfo *formatInfo = inputSource->getFormatInfo();
    AVCodecParameters *videoParameters = avcodec_parameters_alloc();
    avcodec_parameters_copy(videoParameters, formatInfo->vidcodecpar);
    bool mediaCodecSupport = static_cast<bool>(get_mime_by_codec_id((CODEC_TYPE) videoParameters->codec_id));
    if (mediaCodecFirst) {
        if (mediaCodecSupport) {
            videoDecoder = new MediaCodecVideoDecoder();
            videoDecoder->setNativeWindow(nativeWindow);
        } else {
            videoDecoder = new FfmpegVideoDecoder();
        }
    } else {
        videoDecoder = new FfmpegVideoDecoder();
    }
    auto videoDataSize = videoParameters->width * videoParameters->height;
    videoOutFrame = new MediaData(nullptr, static_cast<uint32_t>(videoDataSize),
                                  nullptr, static_cast<uint32_t>(videoDataSize / 4),
                                  nullptr, static_cast<uint32_t>(videoDataSize / 4));
    videoDecoder->init(videoParameters);
    avcodec_parameters_free(&videoParameters);
}

int DecoderHelper::processAudioBuffer(int type, long extra) {
    AVPacket *inPacket = nullptr;
    unsigned long size = inputSource->readAudPkt(&inPacket);
    if (size <= 0) {
        if (stopWhenEmpty) {
            stopWhenEmpty = false;
            return ERROR_EXIST;
        }
        messageSource->pushMessage(MSG_DOMAIN_BUFFER, 0, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        return 0;
    }
    audioLock.lock();
    int mediaSize = audioDecoder->send_packet(inPacket);
    if (mediaSize >= 0) {
        inputSource->popAudPkt(inPacket);
    } else if (mediaSize == INVALID_CODEC) {
        messageSource->pushMessage(MSG_DOMAIN_ERROR, MSG_ERROR_DECODING, 0);
        audioLock.unlock();
        return ERROR_EXIST;
    }
    bool decodeSuccess = audioDecoder->receive_frame(audioOutFrame) >= 0;
    if (!decodeSuccess) {
        audioLock.unlock();
        return 0;
    }
    if (seekPts > 0) {
        if (seekPts <= audioOutFrame->pts) {
            outputSource->pushAudFrame(audioOutFrame);
            seekPts = 0;
        }
    } else {
        outputSource->pushAudFrame(audioOutFrame);
    }
    audioLock.unlock();

    return 0;
}

int DecoderHelper::processVideoBuffer(int type, long extra) {
    AVPacket *inPacket = nullptr;
    unsigned long size = inputSource->readVidPkt(&inPacket);
    if (size <= 0) {
        if (stopWhenEmpty) {
            stopWhenEmpty = false;
            return ERROR_EXIST;
        }
        messageSource->pushMessage(MSG_DOMAIN_BUFFER, 0, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        return 0;
    }
    videoLock.lock();
    int mediaSize = videoDecoder->send_packet(inPacket);
    if (mediaSize >= 0) {
        inputSource->popVidPkt(inPacket);
    } else if (mediaSize == INVALID_CODEC) {
        messageSource->pushMessage(MSG_DOMAIN_ERROR, MSG_ERROR_DECODING, 0);
        videoLock.unlock();
        return ERROR_EXIST;
    }
    bool decodeSuccess = videoDecoder->receive_frame(videoOutFrame) >= 0;
    if (!decodeSuccess) {
        videoLock.unlock();
        return 0;
    }
    if (seekPts > 0) {
        if (seekPts <= videoOutFrame->pts) {
            outputSource->pushVidFrame(videoOutFrame);
            seekPts = 0;
        }
    } else {
        outputSource->pushVidFrame(videoOutFrame);
    }
    videoDecoder->release_buffer();
    videoLock.unlock();

    return mediaSize;
}

void DecoderHelper::onRelease() {
    audioLock.lock();
    videoLock.lock();
    if (!hasInit) {
        videoLock.unlock();
        audioLock.unlock();
        return;
    }
    hasInit = false;
    if (audioDecoder != nullptr) {
        audioDecoder->release();
        delete audioDecoder;
        audioDecoder = nullptr;
    }
    delete audioOutFrame;

    if (videoDecoder != nullptr) {
        videoDecoder->release();
        delete videoDecoder;
        videoDecoder = nullptr;
    }
    delete videoOutFrame;
    videoLock.unlock();
    audioLock.unlock();
}

void DecoderHelper::reset() {
    audioLock.lock();
    videoLock.lock();
    if (hasAudio) audioDecoder->reset();
    if (hasVideo) videoDecoder->reset();
    videoLock.unlock();
    audioLock.unlock();
}
