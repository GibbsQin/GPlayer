//
// Created by Gibbs on 2020/7/21.
//

#include <base/Log.h>
#include "MediaCodecAudioEncoder.h"
#include "CodecUtils.h"

#define TAG "MediaCodecAudioEncoder"

MediaCodecAudioEncoder::MediaCodecAudioEncoder() = default;

MediaCodecAudioEncoder::~MediaCodecAudioEncoder() = default;

void MediaCodecAudioEncoder::init(MediaInfo *header) {
    const char *mine = CodecUtils::codecType2Mime(header->audioType).c_str();
    mAMediaCodec = AMediaCodec_createDecoderByType(mine);
    if (!mAMediaCodec) {
        LOGE(TAG, "can not find mine %s", mine);
        return;
    }

    AMediaFormat *audioFormat = AMediaFormat_new();
    AMediaFormat_setString(audioFormat, AMEDIAFORMAT_KEY_MIME, mine);
    AMediaFormat_setInt32(audioFormat, AMEDIAFORMAT_KEY_SAMPLE_RATE, header->audioSampleRate);
    AMediaFormat_setInt32(audioFormat, AMEDIAFORMAT_KEY_CHANNEL_COUNT, header->audioChannels);
    AMediaFormat_setInt32(audioFormat, AMEDIAFORMAT_KEY_AAC_PROFILE, 2);
    AMediaFormat_setInt32(audioFormat, AMEDIAFORMAT_KEY_IS_ADTS, 1);

    media_status_t status = AMediaCodec_configure(mAMediaCodec, audioFormat, nullptr, nullptr,
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

int MediaCodecAudioEncoder::send_frame(MediaData *inFrame) {
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

int MediaCodecAudioEncoder::receive_packet(MediaData *outPacket) {
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

void MediaCodecAudioEncoder::release() {
    if (mAMediaCodec) {
        AMediaCodec_flush(mAMediaCodec);
        AMediaCodec_stop(mAMediaCodec);
        AMediaCodec_delete(mAMediaCodec);
        mAMediaCodec = nullptr;
    }
}
