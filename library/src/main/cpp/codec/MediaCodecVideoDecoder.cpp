//
// Created by Gibbs on 2020/7/21.
//

#include <cstring>
#include <base/Log.h>
#include "MediaCodecVideoDecoder.h"
extern "C" {
#include <demuxing/avformat_def.h>
}

#define TAG "MediaCodecVideoDecoder"

MediaCodecVideoDecoder::MediaCodecVideoDecoder() = default;

MediaCodecVideoDecoder::~MediaCodecVideoDecoder() = default;

void MediaCodecVideoDecoder::init(AVCodecParameters *codecParameters) {
    const char *mine = getMimeByCodeID((CODEC_TYPE) codecParameters->codec_id);
    mAMediaCodec = AMediaCodec_createDecoderByType(mine);
    if (!mAMediaCodec) {
        LOGE(TAG, "can not find mine %s", mine);
        return;
    }

    mWidth = codecParameters->width;
    mHeight = codecParameters->height;

    AMediaFormat *videoFormat = AMediaFormat_new();
    AMediaFormat_setString(videoFormat, AMEDIAFORMAT_KEY_MIME, mine);
    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_WIDTH, mWidth);
    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_HEIGHT, mHeight);
    media_status_t status = AMediaCodec_configure(mAMediaCodec, videoFormat, nullptr, nullptr, 0);
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

int MediaCodecVideoDecoder::send_packet(MediaData *inPacket) {
    if (!mAMediaCodec) {
        return -1;
    }

    ssize_t bufferId = AMediaCodec_dequeueInputBuffer(mAMediaCodec, 500);
    if (bufferId >= 0) {
        uint32_t flag = 0;
        if ((inPacket->flag & FLAG_KEY_EXTRA_DATA) == FLAG_KEY_EXTRA_DATA) {
            flag = AMEDIACODEC_BUFFER_FLAG_CODEC_CONFIG;
        } else if ((inPacket->flag & FLAG_KEY_FRAME) == FLAG_KEY_FRAME) {
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
        return -1;
    }

    AMediaCodecBufferInfo info;
    ssize_t bufferId = AMediaCodec_dequeueOutputBuffer(mAMediaCodec, &info, 500);
    if (bufferId >= 0) {
        size_t outsize;
        uint8_t *outputBuf = AMediaCodec_getOutputBuffer(mAMediaCodec, bufferId, &outsize);
        if (outputBuf != nullptr) {
            extractFrame(outputBuf, outFrame, info);
            AMediaCodec_releaseOutputBuffer(mAMediaCodec, bufferId, false);
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

void
MediaCodecVideoDecoder::extractFrame(uint8_t *outputBuf, MediaData *outFrame,
                                     AMediaCodecBufferInfo info) {
    outFrame->pts = info.presentationTimeUs;
    outFrame->dts = outFrame->pts;
    uint32_t ySize = mWidth * mHeight;
    uint32_t frameSize = mWidth * mHeight;
    memcpy(outFrame->data, outputBuf, frameSize);
    outFrame->size = frameSize;
    outFrame->size1 = 0;
    outFrame->size2 = 0;
    for (int i = ySize; i < ySize + frameSize / 2; i += 2) {
        outFrame->data1[outFrame->size1++] = outputBuf[i];
        outFrame->data2[outFrame->size2++] = outputBuf[i + 1];
    }
}

void MediaCodecVideoDecoder::release() {
    if (mAMediaCodec) {
        AMediaCodec_flush(mAMediaCodec);
        AMediaCodec_stop(mAMediaCodec);
        AMediaCodec_delete(mAMediaCodec);
        mAMediaCodec = nullptr;
    }
}
