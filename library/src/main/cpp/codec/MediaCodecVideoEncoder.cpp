//
// Created by Gibbs on 2020/7/21.
//

#include <base/Log.h>
#include "MediaCodecVideoEncoder.h"
extern "C" {
#include <demuxing/avformat_def.h>
}

#define TAG "MediaCodecVideoEncoder"

MediaCodecVideoEncoder::MediaCodecVideoEncoder() = default;

MediaCodecVideoEncoder::~MediaCodecVideoEncoder() = default;

void MediaCodecVideoEncoder::init(MediaInfo *header) {
    const char *mine = getMimeByCodeID((CODEC_TYPE) header->videoType);
    mAMediaCodec = AMediaCodec_createDecoderByType(mine);
    if (!mAMediaCodec) {
        LOGE(TAG, "can not find mine %s", mine);
        return;
    }

    AMediaFormat *videoFormat = AMediaFormat_new();
    AMediaFormat_setString(videoFormat, AMEDIAFORMAT_KEY_MIME, mine);
    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_WIDTH, header->videoWidth);
    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_HEIGHT, header->videoHeight);
    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_BIT_RATE, 200000);
    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_FRAME_RATE, header->videoFrameRate);
    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_COLOR_FORMAT, 21);
    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_I_FRAME_INTERVAL, 40);

    media_status_t status = AMediaCodec_configure(mAMediaCodec, videoFormat, nullptr, nullptr,
                                                  AMEDIACODEC_CONFIGURE_FLAG_ENCODE);
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
}

int MediaCodecVideoEncoder::send_frame(MediaData *inFrame) {
    if (!mAMediaCodec) {
        return -1;
    }

    ssize_t bufferId = AMediaCodec_dequeueInputBuffer(mAMediaCodec, 2000);
    if (bufferId >= 0) {
        // 获取buffer的索引
        size_t outsize;
        uint8_t *inputBuf = AMediaCodec_getInputBuffer(mAMediaCodec, bufferId, &outsize);
        if (inputBuf != nullptr && inFrame->size <= outsize) {
            // 将待解码的数据copy到硬件中
            memcpy(inputBuf, inFrame->data, inFrame->size);
            media_status_t status = AMediaCodec_queueInputBuffer(mAMediaCodec, bufferId, 0,
                                                                 inFrame->size, inFrame->pts, 0);
            return (status == AMEDIA_OK ? 0 : -2);
        }
    }
    return -3;
}

int MediaCodecVideoEncoder::receive_packet(MediaData *outPacket) {
    if (!mAMediaCodec) {
        return -1;
    }

    AMediaCodecBufferInfo info;
    ssize_t bufferId = AMediaCodec_dequeueOutputBuffer(mAMediaCodec, &info, 2000);
    if (bufferId >= 0) {
        size_t outsize;
        uint8_t *outputBuf = AMediaCodec_getOutputBuffer(mAMediaCodec, bufferId, &outsize);
        if (outputBuf != nullptr) {
            memcpy(outPacket->data, outputBuf, info.size);
            outPacket->size = info.size;
            outPacket->pts = info.presentationTimeUs;
            outPacket->flag = (info.flags & 0x01);
            AMediaCodec_releaseOutputBuffer(mAMediaCodec, bufferId, false);
            return 0;
        }
    }
    return -3;
}

void MediaCodecVideoEncoder::release() {
    if (mAMediaCodec) {
        AMediaCodec_flush(mAMediaCodec);
        AMediaCodec_stop(mAMediaCodec);
        AMediaCodec_delete(mAMediaCodec);
        mAMediaCodec = nullptr;
    }
}
