/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#include <cstring>
#include <base/Log.h>
#include "MediaCodecVideoDecoder.h"

extern "C" {
#include <demuxing/avformat_def.h>
}

#define TAG "MediaCodecVideoDecoder"

MediaCodecVideoDecoder::MediaCodecVideoDecoder() {
    nativeWindow = nullptr;
    mAMediaCodec = nullptr;
}

MediaCodecVideoDecoder::~MediaCodecVideoDecoder() = default;

void MediaCodecVideoDecoder::init(AVCodecParameters *codecParameters) {
    const char *mine = get_mime_by_codec_id((CODEC_TYPE) codecParameters->codec_id);
    mAMediaCodec = AMediaCodec_createDecoderByType(mine);
    if (!mAMediaCodec) {
        LOGE(TAG, "can not find mine %s", mine);
        return;
    }
    if (nativeWindow == nullptr) {
        LOGE(TAG, "must set native window first");
        return;
    }

    mWidth = codecParameters->width;
    mHeight = codecParameters->height;
    LOGI(TAG, "CoreFlow : init mine=%s,width=%d,height=%d", mine, mWidth, mHeight);
    if (mWidth == 0 || mHeight == 0) {
        LOGE(TAG, "video size is invalid");
        return;
    }

    AMediaFormat *videoFormat = AMediaFormat_new();
    AMediaFormat_setString(videoFormat, AMEDIAFORMAT_KEY_MIME, mine);
    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_WIDTH, mWidth);
    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_HEIGHT, mHeight);
    AMediaFormat_setBuffer(videoFormat, "csd-0", codecParameters->extradata,
                           codecParameters->extradata_size);
    media_status_t status = AMediaCodec_configure(mAMediaCodec, videoFormat, nativeWindow, nullptr, 0);
    if (status != AMEDIA_OK) {
        LOGE(TAG, "configure fail %d", status);
        AMediaCodec_delete(mAMediaCodec);
        mAMediaCodec = nullptr;
        return;
    }

    status = AMediaCodec_start(mAMediaCodec);
    if (status != AMEDIA_OK) {
        LOGE(TAG, "start fail %d", status);
        AMediaCodec_delete(mAMediaCodec);
        mAMediaCodec = nullptr;
        return;
    }
    LOGI(TAG, "start successfully");
}

int MediaCodecVideoDecoder::send_packet(AVPacket *inPacket) {
    if (!mAMediaCodec) {
        return INVALID_CODEC;
    }

    ssize_t bufferId = AMediaCodec_dequeueInputBuffer(mAMediaCodec, 500);
    if (bufferId >= 0) {
        uint32_t flag = 0;
        if ((inPacket->flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY) {
            flag = AMEDIACODEC_BUFFER_FLAG_PARTIAL_FRAME;
        }
        // 获取buffer的索引
        size_t outsize;
        uint8_t *inputBuf = AMediaCodec_getInputBuffer(mAMediaCodec, bufferId, &outsize);
        if (inputBuf != nullptr && inPacket->size <= outsize) {
            // 将待解码的数据copy到硬件中
            memcpy(inputBuf, inPacket->data, inPacket->size);
            media_status_t status = AMediaCodec_queueInputBuffer(mAMediaCodec, bufferId, 0,
                                                                 inPacket->size, inPacket->pts,
                                                                 flag);
            return (status == AMEDIA_OK ? 0 : -2);
        }
    }
    return TRY_AGAIN;
}

int MediaCodecVideoDecoder::receive_frame(MediaData *outFrame) {
    if (!mAMediaCodec) {
        return INVALID_CODEC;
    }

    AMediaCodecBufferInfo info;
    ssize_t bufferId = AMediaCodec_dequeueOutputBuffer(mAMediaCodec, &info, 500);
    if (bufferId >= 0) {
        size_t outsize;
        uint8_t *outputBuf = AMediaCodec_getOutputBuffer(mAMediaCodec, bufferId, &outsize);
        if (outputBuf != nullptr) {
            outFrame->pts = info.presentationTimeUs;
            outFrame->dts = outFrame->pts;
            mCurrentBufferId = bufferId;
            return 0;
        }
    } else if (bufferId == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
        auto format = AMediaCodec_getOutputFormat(mAMediaCodec);
        AMediaFormat_getInt32(format, "width", &mWidth);
        AMediaFormat_getInt32(format, "height", &mHeight);
        int32_t localColorFMT;
        AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, &localColorFMT);
        LOGI(TAG, "format changed %s", AMediaFormat_toString(format));
        return -2;
    } else if (bufferId == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {

    }
    return -3;
}

void MediaCodecVideoDecoder::release_buffer() {
    AMediaCodec_releaseOutputBuffer(mAMediaCodec, mCurrentBufferId, true);
}

void MediaCodecVideoDecoder::release() {
    if (mAMediaCodec) {
        AMediaCodec_flush(mAMediaCodec);
        AMediaCodec_stop(mAMediaCodec);
        AMediaCodec_delete(mAMediaCodec);
        mAMediaCodec = nullptr;
    }
}

void MediaCodecVideoDecoder::reset() {
    AMediaCodec_flush(mAMediaCodec);
}
