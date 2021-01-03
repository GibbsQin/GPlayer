/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#include "base/Log.h"
#include "FfmpegAudioDecoder.h"

#define TAG "FfmpegAudioDecoder"

FfmpegAudioDecoder::FfmpegAudioDecoder() = default;

FfmpegAudioDecoder::~FfmpegAudioDecoder() = default;

void FfmpegAudioDecoder::init(AVCodecParameters *codecParameters) {
    LOGI(TAG, "CoreFlow : init");
    av_register_all();

    AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
    mCodecContext = avcodec_alloc_context3(codec);
    int ret = avcodec_parameters_from_context(codecParameters, mCodecContext);
    if (ret < 0) {
        LOGE(TAG, "Error: avcodec_parameters_from_context code is %d %s", ret, av_err2str(ret));
        return;
    }

    ret = avcodec_open2(mCodecContext, codec, nullptr);
    if (ret < 0) {
        LOGE(TAG, "Error: avcodec_open2 code is %d %s", ret, av_err2str(ret));
        return;
    }

    mOutFrame = av_frame_alloc();
    isInitSuccess = true;
}

int FfmpegAudioDecoder::send_packet(AVPacket *inPacket) {
    if (!isInitSuccess) {
        LOGE(TAG, "Error: decoder init error");
        return INVALID_CODEC;
    }

    if (inPacket == nullptr) {
        LOGE(TAG, "Error: decode the param is nullptr");
        return -1;
    }

    int ret = avcodec_send_packet(mCodecContext, inPacket);
    if (ret < 0) {
        if (ret != AVERROR_EOF) {
            LOGE(TAG, "Error: avcodec_send_packet %d %s %d", ret, av_err2str(ret), inPacket->size);
        }
        return ret;
    }

    return 0;
}

int FfmpegAudioDecoder::receive_frame(MediaData *outFrame) {
    if (!isInitSuccess) {
        LOGE(TAG, "Error: decoder init error");
        return INVALID_CODEC;
    }

    if (outFrame == nullptr) {
        LOGE(TAG, "Error: decode the param is nullptr");
        return -1;
    }
    outFrame->size = 0;

    int data_size = av_get_bytes_per_sample(mCodecContext->sample_fmt);
    if (data_size < 0) {
        /* This should not occur, checking just for paranoia */
        LOGE(TAG, "Error: Failed to calculate data size\n");
        return -3;
    }

    int ret = 0;
    int totalSize = 0;
    int i;
    int ch;
    while (ret >= 0) {
        ret = avcodec_receive_frame(mCodecContext, mOutFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            LOGE(TAG, "Error: avcodec_receive_frame %d %s", ret, av_err2str(ret));
            break;
        }
        for (i = 0; i < mOutFrame->nb_samples; i++) {
            for (ch = 0; ch < mCodecContext->channels; ch++) {
                memcpy(outFrame->data + totalSize, mOutFrame->data[ch] + data_size * i,
                       static_cast<size_t>(data_size));
                totalSize += data_size;
            }
        }
        outFrame->size = static_cast<uint32_t>(totalSize);
        outFrame->pts = static_cast<uint64_t>(mOutFrame->pts);
        outFrame->dts = outFrame->pts;
        av_frame_unref(mOutFrame);
    }

    return (outFrame->size > 0) ? 0 : -4;
}

void FfmpegAudioDecoder::release() {
    LOGI(TAG, "release");
    if (mOutFrame != nullptr) {
        av_frame_free(&mOutFrame);
        mOutFrame = nullptr;
    }

    if (mCodecContext != nullptr) {
        avcodec_close(mCodecContext);
        avcodec_free_context(&mCodecContext);
        mCodecContext = nullptr;
    }
}

void FfmpegAudioDecoder::reset() {
    avcodec_flush_buffers(mCodecContext);
}
