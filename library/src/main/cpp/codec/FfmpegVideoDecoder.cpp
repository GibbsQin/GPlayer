#include "base/Log.h"
#include "FfmpegVideoDecoder.h"

FfmpegVideoDecoder::FfmpegVideoDecoder() = default;

FfmpegVideoDecoder::~FfmpegVideoDecoder() = default;

void FfmpegVideoDecoder::init(AVCodecParameters *codecParameters) {
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

int FfmpegVideoDecoder::send_packet(AVPacket *inPacket) {
    if (inPacket == nullptr) {
        LOGE(TAG, "Error: decode the param is nullptr");
        return -1;
    }

    if (!isInitSuccess) {
        LOGE(TAG, "Error: decoder init error");
        return -2;
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

int FfmpegVideoDecoder::receive_frame(MediaData *outFrame) {
    if (outFrame == nullptr) {
        LOGE(TAG, "Error: decode the param is nullptr");
        return -1;
    }

    if (!isInitSuccess) {
        LOGE(TAG, "Error: decoder init error");
        return -2;
    }

    int ret = avcodec_receive_frame(mCodecContext, mOutFrame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return ret;
    } else if (ret < 0) {
        LOGE(TAG, "Error: avcodec_receive_frame %d %s", ret, av_err2str(ret));
        return ret;
    }

    copy_mediadata_from_frame(outFrame, mOutFrame);

    return 0;
}

void FfmpegVideoDecoder::release() {
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

void FfmpegVideoDecoder::copy_mediadata_from_frame(MediaData *mediaData, AVFrame *frame) {
    //处理线宽
    if (frame->linesize[0] > frame->width) {
        uint8_t *srcY = frame->data[0];
        uint8_t *srcU = frame->data[1];
        uint8_t *srcV = frame->data[2];
        uint8_t *dstY = mediaData->data;
        uint8_t *dstU = mediaData->data1;
        uint8_t *dstV = mediaData->data2;
        for (int i = 0; i < frame->height; i++) {
            memcpy(dstY, srcY, static_cast<size_t>(frame->width));
            dstY += frame->width;
            srcY += frame->linesize[0];
        }
        for (int i = 0; i < (frame->height / 2); i++) {
            memcpy(dstU, srcU, static_cast<size_t>(frame->width / 2));
            dstU += (frame->width / 2);
            srcU += frame->linesize[1];
            memcpy(dstV, srcV, static_cast<size_t>(frame->width / 2));
            dstV += (frame->width / 2);
            srcV += frame->linesize[2];
        }
        auto dataSize = frame->height * frame->width;
        mediaData->size = static_cast<uint32_t>(dataSize);
        mediaData->size1 = static_cast<uint32_t>(dataSize / 4);
        mediaData->size2 = static_cast<uint32_t>(dataSize / 4);
    } else {
        mediaData->size = static_cast<uint32_t>(frame->height * frame->width);
        mediaData->size1 = mediaData->size / 4;
        mediaData->size2 = mediaData->size1;
        memcpy(mediaData->data, frame->data[0], mediaData->size);
        memcpy(mediaData->data1, frame->data[1], mediaData->size1);
        memcpy(mediaData->data2, frame->data[2], mediaData->size2);
    }

    mediaData->width = static_cast<uint32_t>(frame->width);
    mediaData->height = static_cast<uint32_t>(frame->height);
    mediaData->pts = static_cast<uint64_t>(frame->pts);
    mediaData->dts = mediaData->dts;
    mediaData->flag = static_cast<uint8_t>(frame->key_frame);
}
