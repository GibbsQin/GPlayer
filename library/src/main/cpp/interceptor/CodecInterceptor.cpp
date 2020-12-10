//
// Created by Gibbs on 2020/7/18.
//

#include <codec/FfmpegAudioDecoder.h>
#include <codec/FfmpegVideoDecoder.h>
#include <base/Log.h>
#include <codec/MediaCodecVideoDecoder.h>
#include <codec/MediaCodecAudioDecoder.h>
#include "CodecInterceptor.h"

extern "C" {
#include <demuxing/avformat_def.h>
}

#define TAG "CodecInterceptor"

CodecInterceptor::CodecInterceptor(bool mediaCodecFirst) {
    this->mediaCodecFirst = mediaCodecFirst;
    hasInit = false;
}

CodecInterceptor::~CodecInterceptor() = default;

int CodecInterceptor::onInit(FormatInfo *formatInfo) {
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
    bool mediaCodecSupport = get_mime_by_codec_id((CODEC_TYPE) audioParameters->codec_id);
    if (mediaCodecFirst) {
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
    mediaCodecSupport = get_mime_by_codec_id((CODEC_TYPE) audioParameters->codec_id);
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

int CodecInterceptor::inputBuffer(AVPacket *buffer, int type) {
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

int CodecInterceptor::outputBuffer(MediaData **buffer, int type) {
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

void CodecInterceptor::onRelease() {
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
