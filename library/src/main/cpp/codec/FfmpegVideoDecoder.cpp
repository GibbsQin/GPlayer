#include <base/Log.h>
#include "FfmpegVideoDecoder.h"

FfmpegVideoDecoder::FfmpegVideoDecoder() {
    isInitSuccess = false;
    mCodec = nullptr;
    mCodecContext = nullptr;
    mOutFrame = nullptr;
}

FfmpegVideoDecoder::~FfmpegVideoDecoder() = default;

void FfmpegVideoDecoder::init(AVCodecParameters *codecParameters) {
    av_register_all();

    mCodec = avcodec_find_decoder(codecParameters->codec_id);
    mCodecContext = avcodec_alloc_context3(mCodec);
    avcodec_parameters_from_context(codecParameters, mCodecContext);

    if (avcodec_open2(mCodecContext, mCodec, nullptr) < 0) {
        LOGE(TAG, "could not open encode-codec");
        return;
    }

    mOutFrame = av_frame_alloc();

    isInitSuccess = true;
}

int FfmpegVideoDecoder::send_packet(AVPacket *inPacket) {
    if (inPacket == nullptr) {
        LOGE(TAG, "decode the param is nullptr");
        return -1;
    }

    if (!isInitSuccess) {
        LOGE(TAG, "decoder init error");
        return -2;
    }

    int result = avcodec_send_packet(mCodecContext, inPacket);
    if (result < 0) {
        LOGE(TAG, "Error: avcodec_send_packet %d %s", result, av_err2str(result));
        return result;
    }
    av_packet_unref(inPacket);

    return 0;
}

int FfmpegVideoDecoder::receive_frame(MediaData *outFrame) {
    if (outFrame == nullptr) {
        LOGE(TAG, "decode the param is nullptr");
        return -1;
    }

    if (!isInitSuccess) {
        LOGE(TAG, "decoder init error");
        return -2;
    }

    int result = avcodec_receive_frame(mCodecContext, mOutFrame);
    if (result < 0) {
        LOGE(TAG, "Error: avcodec_receive_frame %d %s", result, av_err2str(result));
        return result;
    }

    //处理线宽
    if (mOutFrame->linesize[0] > mOutFrame->width) {
        uint8_t *srcY = mOutFrame->data[0];
        uint8_t *srcU = mOutFrame->data[1];
        uint8_t *srcV = mOutFrame->data[2];
        uint8_t *dstY = outFrame->data;
        uint8_t *dstU = outFrame->data1;
        uint8_t *dstV = outFrame->data2;
        for (int i = 0; i < mOutFrame->height; i++) {
            memcpy(dstY, srcY, static_cast<size_t>(mOutFrame->width));
            dstY += mOutFrame->width;
            srcY += mOutFrame->linesize[0];
        }
        for (int i = 0; i < (mOutFrame->height / 2); i++) {
            memcpy(dstU, srcU, static_cast<size_t>(mOutFrame->width / 2));
            dstU += (mOutFrame->width / 2);
            srcU += mOutFrame->linesize[1];
            memcpy(dstV, srcV, static_cast<size_t>(mOutFrame->width / 2));
            dstV += (mOutFrame->width / 2);
            srcV += mOutFrame->linesize[2];
        }
        auto dataSize = mOutFrame->height * mOutFrame->width;
        outFrame->size = dataSize;
        outFrame->size1 = dataSize / 4;
        outFrame->size2 = dataSize / 4;
    } else {
        outFrame->size = static_cast<uint32_t>(mOutFrame->height * mOutFrame->width);
        outFrame->size1 = outFrame->size / 4;
        outFrame->size2 = outFrame->size1;
        memcpy(outFrame->data, mOutFrame->data[0], outFrame->size);
        memcpy(outFrame->data1, mOutFrame->data[1], outFrame->size1);
        memcpy(outFrame->data2, mOutFrame->data[2], outFrame->size2);
    }

    outFrame->width = static_cast<uint32_t>(mOutFrame->width);
    outFrame->height = static_cast<uint32_t>(mOutFrame->height);
    outFrame->pts = static_cast<uint64_t>(mOutFrame->pts);
    outFrame->dts = outFrame->dts;
    outFrame->flag = static_cast<uint8_t>(mOutFrame->key_frame);

    return 0;
}

void FfmpegVideoDecoder::release() {
    LOGI(TAG, "release");

    if (mOutFrame != nullptr) {
        av_frame_free(&mOutFrame);
        mOutFrame = nullptr;
    }

    if (mCodecContext != nullptr) {
        // Close the codec
        avcodec_close(mCodecContext);
        avcodec_free_context(&mCodecContext);
        mCodecContext = nullptr;
    }
}