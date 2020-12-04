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
    isAudioAvailable = false;
    isVideoAvailable = false;
}

CodecInterceptor::~CodecInterceptor() = default;

int CodecInterceptor::onInit(MediaInfo *header) {
    audioLock.lock();
    videoLock.lock();
    if (hasInit) {
        videoLock.unlock();
        audioLock.unlock();
        return -1;
    }
    hasInit = true;
    bool ffmpegSupport = header->audioType > CODEC_START && header->audioType < CODEC_END;
    bool mediaCodecSupport = getMimeByCodeID((CODEC_TYPE) header->audioType) != "";
    LOGI(TAG, "CoreFlow : onInit ffmpegSupport %d, mediaCodecSupport = %d, mediaCodecFirst = %d",
         ffmpegSupport, mediaCodecSupport, mediaCodecFirst);
    isAudioAvailable = (ffmpegSupport || mediaCodecSupport);
    if (isAudioAvailable) {
        if (mediaCodecFirst) {
            if (mediaCodecSupport) {
                audioDecoder = new MediaCodecAudioDecoder();
                LOGI(TAG, "new MediaCodecAudioDecoder");
            } else {
                audioDecoder = new FfmpegAudioDecoder();
                LOGI(TAG, "new FfmpegAudioDecoder");
            }
        } else {
            if (ffmpegSupport) {
                audioDecoder = new FfmpegAudioDecoder();
                LOGI(TAG, "new FfmpegAudioDecoder");
            } else {
                audioDecoder = new MediaCodecAudioDecoder();
                LOGI(TAG, "new MediaCodecAudioDecoder");
            }
        }
        auto audioDataSize =
                header->sampleNumPerFrame * header->audioBitWidth * header->audioChannels;
        audioOutFrame = new MediaData(nullptr, static_cast<uint32_t>(audioDataSize), nullptr, 0,
                                      nullptr, 0);
        audioDecoder->init(header);
    }

    ffmpegSupport = header->videoType > CODEC_START && header->videoType < CODEC_END;
    mediaCodecSupport = getMimeByCodeID((CODEC_TYPE) header->videoType) != "";
    isVideoAvailable = (ffmpegSupport || mediaCodecSupport);
    if (isVideoAvailable) {
        if (mediaCodecFirst) {
            if (mediaCodecSupport) {
                videoDecoder = new MediaCodecVideoDecoder();
                LOGI(TAG, "new MediaCodecVideoDecoder");
            } else {
                videoDecoder = new FfmpegVideoDecoder();
                LOGI(TAG, "new FfmpegVideoDecoder");
            }
        } else {
            if (ffmpegSupport) {
                videoDecoder = new FfmpegVideoDecoder();
                LOGI(TAG, "new FfmpegVideoDecoder");
            } else {
                videoDecoder = new MediaCodecVideoDecoder();
                LOGI(TAG, "new MediaCodecVideoDecoder");
            }
        }
        auto videoDataSize = header->videoWidth * header->videoHeight;
        videoOutFrame = new MediaData(nullptr, static_cast<uint32_t>(videoDataSize),
                                      nullptr, static_cast<uint32_t>(videoDataSize / 4),
                                      nullptr, static_cast<uint32_t>(videoDataSize / 4));
        videoDecoder->init(header);
    }
    videoLock.unlock();
    audioLock.unlock();
    if (!isVideoAvailable || !isAudioAvailable) {
        return 1;
    }

    return 0;
}

int CodecInterceptor::inputBuffer(MediaData *buffer, int type) {
    int ret = -1;
    if (type == AV_TYPE_AUDIO) {
        audioLock.lock();
        if (isAudioAvailable) {
            ret = audioDecoder->send_packet(buffer);
        }
        audioLock.unlock();
    } else if (type == AV_TYPE_VIDEO) {
        videoLock.lock();
        if (isVideoAvailable) {
            ret = videoDecoder->send_packet(buffer);
        }
        videoLock.unlock();
    }
    return ret;
}

int CodecInterceptor::outputBuffer(MediaData **buffer, int type) {
    int ret = -1;
    if (type == AV_TYPE_AUDIO) {
        audioLock.lock();
        if (isAudioAvailable) {
            ret = audioDecoder->receive_frame(audioOutFrame);
            *buffer = audioOutFrame;
        }
        audioLock.unlock();
    } else if (type == AV_TYPE_VIDEO) {
        videoLock.lock();
        if (isVideoAvailable) {
            ret = videoDecoder->receive_frame(videoOutFrame);
            *buffer = videoOutFrame;
        }
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
    if (isAudioAvailable) {
        if (audioDecoder != nullptr) {
            audioDecoder->release();
            delete audioDecoder;
            audioDecoder = nullptr;
        }
        delete audioOutFrame;
    }

    if (isVideoAvailable) {
        if (videoDecoder != nullptr) {
            videoDecoder->release();
            delete videoDecoder;
            videoDecoder = nullptr;
        }
        delete videoOutFrame;
    }
    videoLock.unlock();
    audioLock.unlock();
}
