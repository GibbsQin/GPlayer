//
// Created by Gibbs on 2020/7/18.
//

#include <codec/FfmpegAudioDecoder.h>
#include <codec/FfmpegVideoDecoder.h>
#include <base/Log.h>
#include <codec/MediaCodecVideoDecoder.h>
#include <codec/MediaCodecAudioDecoder.h>
#include "DecoderHelper.h"

extern "C" {
#include <demuxing/avformat_def.h>
}

#define TAG "CodecInterceptor"

DecoderHelper::DecoderHelper(PacketSource *input, FrameSource *output, MessageQueue *messageQueue, bool mediaCodecFirst) {
    inputSource = input;
    outputSource = output;
    this->mediaCodecFirst = mediaCodecFirst;
    this->messageQueue = messageQueue;
    hasInit = false;
}

DecoderHelper::~DecoderHelper() = default;

int DecoderHelper::onInit() {
    FormatInfo *formatInfo = inputSource->getFormatInfo();
    audioLock.lock();
    videoLock.lock();
    if (hasInit) {
        videoLock.unlock();
        audioLock.unlock();
        return -1;
    }
    hasInit = true;

    AVCodecParameters *audioParameters = avcodec_parameters_alloc();
    avcodec_parameters_copy(audioParameters, formatInfo->audcodecpar);
    bool mediaCodecSupport = static_cast<bool>(get_mime_by_codec_id(
            (CODEC_TYPE) audioParameters->codec_id));
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

    AVCodecParameters *videoParameters = avcodec_parameters_alloc();
    avcodec_parameters_copy(videoParameters, formatInfo->vidcodecpar);
    mediaCodecSupport = static_cast<bool>(get_mime_by_codec_id(
            (CODEC_TYPE) audioParameters->codec_id));
    if (mediaCodecFirst) {
        if (mediaCodecSupport) {
            videoDecoder = new MediaCodecVideoDecoder();
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

    videoLock.unlock();
    audioLock.unlock();

    avcodec_parameters_free(&audioParameters);
    avcodec_parameters_free(&videoParameters);
    return 0;
}

int DecoderHelper::processAudioBuffer(int type, long extra) {
    AVPacket *inPacket = nullptr;
    int ret = inputSource->dequeAudPkt(&inPacket);
    if (ret <= 0) {
        return 0;
    }
    int mediaSize = 0;
    int inputResult = inputBuffer(inPacket, AV_TYPE_AUDIO);
    MediaData *outBuffer = nullptr;
    ret = outputBuffer(&outBuffer, AV_TYPE_AUDIO);
    if (ret >= 0) {
        mediaSize = outputSource->onReceiveAudio(outBuffer);
    }
    if (inputResult >= 0) {
        inputSource->popAudPkt(inPacket);
    }

    return mediaSize;
}

int DecoderHelper::processVideoBuffer(int type, long extra) {
    AVPacket *inPacket = nullptr;
    int ret = inputSource->dequeVidPkt(&inPacket);
    if (ret <= 0) {
        return 0;
    }
    int mediaSize = 0;
    int inputResult = inputBuffer(inPacket, AV_TYPE_VIDEO);
    MediaData *outBuffer = nullptr;
    ret = outputBuffer(&outBuffer, AV_TYPE_VIDEO);
    if (ret >= 0) {
        mediaSize = outputSource->onReceiveVideo(outBuffer);
    }
    if (inputResult >= 0) {
        inputSource->popVidPkt(inPacket);
    }

    return mediaSize;
}

int DecoderHelper::inputBuffer(AVPacket *buffer, int type) {
    int ret = -1;
    if (type == AV_TYPE_AUDIO) {
        audioLock.lock();
        ret = audioDecoder->send_packet(buffer);
        audioLock.unlock();
    } else if (type == AV_TYPE_VIDEO) {
        videoLock.lock();
        ret = videoDecoder->send_packet(buffer);
        videoLock.unlock();
    }
    return ret;
}

int DecoderHelper::outputBuffer(MediaData **buffer, int type) {
    int ret = -1;
    if (type == AV_TYPE_AUDIO) {
        audioLock.lock();
        ret = audioDecoder->receive_frame(audioOutFrame);
        *buffer = audioOutFrame;
        audioLock.unlock();
    } else if (type == AV_TYPE_VIDEO) {
        videoLock.lock();
        ret = videoDecoder->receive_frame(videoOutFrame);
        *buffer = videoOutFrame;
        videoLock.unlock();
    }
    return ret;
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

void DecoderHelper::enableMediaCodec() {
    mediaCodecFirst = true;
}
