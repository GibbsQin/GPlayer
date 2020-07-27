//
// Created by Gibbs on 2020/7/21.
//

#include <cstring>
#include <base/Log.h>
#include "MediaCodecAudioDecoder.h"
#include "CodecUtils.h"

#define TAG "MediaCodecAudioDecoder"

MediaCodecAudioDecoder::MediaCodecAudioDecoder() = default;

MediaCodecAudioDecoder::~MediaCodecAudioDecoder() = default;

void MediaCodecAudioDecoder::init(MediaInfo *header) {
    const char *mine = CodecUtils::codecType2Mime(header->audioType).c_str();
    mAMediaCodec = AMediaCodec_createDecoderByType(mine);
    if (!mAMediaCodec) {
        LOGE(TAG, "can not find mine %s", mine);
        return;
    }

    mHeader = header;
    mHeader->audioBitWidth = 2;
    int sampleRate = mHeader->audioSampleRate;
    int channelCount = mHeader->audioChannels;
    int profile = 2;
    LOGI(TAG, "init mine=%s,sampleRate=%d,channelCount=%d", mine, sampleRate, channelCount);

    AMediaFormat *audioFormat = AMediaFormat_new();
    AMediaFormat_setString(audioFormat, AMEDIAFORMAT_KEY_MIME, mine);
    AMediaFormat_setInt32(audioFormat, AMEDIAFORMAT_KEY_SAMPLE_RATE, sampleRate);
    AMediaFormat_setInt32(audioFormat, AMEDIAFORMAT_KEY_CHANNEL_COUNT, channelCount);
    AMediaFormat_setInt32(audioFormat, AMEDIAFORMAT_KEY_AAC_PROFILE, profile);
    AMediaFormat_setInt32(audioFormat, AMEDIAFORMAT_KEY_IS_ADTS, 1);

    media_status_t status = AMediaCodec_configure(mAMediaCodec, audioFormat, nullptr, nullptr, 0);
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

int MediaCodecAudioDecoder::send_packet(MediaData *inPacket) {
    if (!mAMediaCodec) {
        return -1;
    }

    ssize_t bufferId = AMediaCodec_dequeueInputBuffer(mAMediaCodec, 2000);
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
                                                                 inPacket->size, inPacket->pts, flag);
            return (status == AMEDIA_OK ? 0 : -2);
        }
    }
    return TRY_AGAIN;
}

int MediaCodecAudioDecoder::receive_frame(MediaData *outFrame) {
    if (!mAMediaCodec) {
        return -1;
    }

    AMediaCodecBufferInfo info;
    ssize_t bufferId = AMediaCodec_dequeueOutputBuffer(mAMediaCodec, &info, 2000);
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
        int32_t localColorFMT;
        AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, &localColorFMT);
        return -2;
    } else if (bufferId == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {

    }
    return -3;
}

void
MediaCodecAudioDecoder::extractFrame(uint8_t *outputBuf, MediaData *outFrame, AMediaCodecBufferInfo info) {
    outFrame->pts = info.presentationTimeUs;
    outFrame->dts = outFrame->pts;
    uint32_t frameSize = info.size;
    memcpy(outFrame->data, outputBuf, frameSize);
    outFrame->size = frameSize;
}

void MediaCodecAudioDecoder::release() {
    if (mAMediaCodec) {
        AMediaCodec_flush(mAMediaCodec);
        AMediaCodec_stop(mAMediaCodec);
        AMediaCodec_delete(mAMediaCodec);
        mAMediaCodec = nullptr;
    }
}
