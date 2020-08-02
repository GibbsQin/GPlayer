//
// Created by Gibbs on 2020/7/18.
//

#include <codec/FfmpegAudioDecoder.h>
#include <codec/FfmpegVideoDecoder.h>
#include <codec/CodecUtils.h>
#include <base/Log.h>
#include <codec/MediaCodecVideoDecoder.h>
#include <codec/MediaCodecAudioDecoder.h>
#include "CodecInterceptor.h"

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
    bool ffmpegSupport = CodecUtils::codecType2CodecId(header->audioType) > 0;
    bool mediaCodecSupport = (!CodecUtils::codecType2Mime(header->audioType).empty());
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
        auto audioDataSize = header->sampleNumPerFrame * header->audioBitWidth * header->audioChannels;
        audioOutFrame.data = (uint8_t *) malloc(audioDataSize);
        audioOutFrame.data1 = nullptr;
        audioOutFrame.data2 = nullptr;
        audioOutFrame.size = 0;
        audioOutFrame.size1 = 0;
        audioOutFrame.size2 = 0;
        audioDecoder->init(header);
    } else {
        audioOutFrame.data = nullptr;
        audioOutFrame.data1 = nullptr;
        audioOutFrame.data2 = nullptr;
        audioOutFrame.size = 0;
        audioOutFrame.size1 = 0;
        audioOutFrame.size2 = 0;
    }

    ffmpegSupport = CodecUtils::codecType2CodecId(header->videoType) > 0;
    mediaCodecSupport = (!CodecUtils::codecType2Mime(header->videoType).empty());
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
        videoOutFrame.data = (uint8_t *) malloc(videoDataSize);
        videoOutFrame.data1 = (uint8_t *) malloc(videoDataSize / 4);
        videoOutFrame.data2 = (uint8_t *) malloc(videoDataSize / 4);
        videoOutFrame.size = 0;
        videoOutFrame.size1 = 0;
        videoOutFrame.size2 = 0;
        videoDecoder->init(header);
    } else {
        videoOutFrame.data = nullptr;
        videoOutFrame.data1 = nullptr;
        videoOutFrame.data2 = nullptr;
        videoOutFrame.size = 0;
        videoOutFrame.size1 = 0;
        videoOutFrame.size2 = 0;
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
            ret = audioDecoder->receive_frame(&audioOutFrame);
            *buffer = &audioOutFrame;
        }
        audioLock.unlock();
    } else if (type == AV_TYPE_VIDEO) {
        videoLock.lock();
        if (isVideoAvailable) {
            ret = videoDecoder->receive_frame(&videoOutFrame);
            *buffer = &videoOutFrame;
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
        if (audioOutFrame.data) {
            free(audioOutFrame.data);
            audioOutFrame.data = nullptr;
        }
    }

    if (isVideoAvailable) {
        if (videoDecoder != nullptr) {
            videoDecoder->release();
            delete videoDecoder;
            videoDecoder = nullptr;
        }
        if (videoOutFrame.data) {
            free(videoOutFrame.data);
            videoOutFrame.data = nullptr;
        }
        if (videoOutFrame.data1) {
            free(videoOutFrame.data1);
            videoOutFrame.data1 = nullptr;
        }
        if (videoOutFrame.data2) {
            free(videoOutFrame.data2);
            videoOutFrame.data2 = nullptr;
        }
    }
    videoLock.unlock();
    audioLock.unlock();
    LOGI(TAG, "CoreFlow : onRelease");
}
