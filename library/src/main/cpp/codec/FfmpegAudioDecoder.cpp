
#include "base/Log.h"
#include "FfmpegAudioDecoder.h"

FfmpegAudioDecoder::FfmpegAudioDecoder() {
    isInitSuccess = false;
    mCodec = nullptr;
    mCodecContext = nullptr;
    mOutFrame = nullptr;
}

FfmpegAudioDecoder::~FfmpegAudioDecoder() = default;

void FfmpegAudioDecoder::init(AVCodecParameters *codecParameters) {
    av_register_all();
    mCodec = avcodec_find_decoder(codecParameters->codec_id);
    mCodecContext = avcodec_alloc_context3(mCodec);
    avcodec_parameters_from_context(codecParameters, mCodecContext);

    LOGI(TAG, "init sample_fmt=%d,sample_rate=%d,channel_layout=%lld,channels=%d,frame_size=%d",
         mCodecContext->sample_fmt, mCodecContext->sample_rate, mCodecContext->channel_layout,
         mCodecContext->channels, mCodecContext->frame_size);

    int ret = avcodec_open2(mCodecContext, mCodec, nullptr);
    if (ret < 0) {
        LOGE(TAG, "Error: avcodec_open2 code is %d", ret);
        return;
    }

    mOutFrame = av_frame_alloc();
    isInitSuccess = true;
}

int FfmpegAudioDecoder::send_packet(AVPacket *inPacket) {
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
        LOGE(TAG, "Error: avcodec_send_packet %d %s", ret, av_err2str(ret));
        return ret;
    }
    av_packet_unref(inPacket);

    return 0;
}

int FfmpegAudioDecoder::receive_frame(MediaData *outFrame) {
    if (outFrame == nullptr) {
        LOGE(TAG, "Error: decode the param is nullptr");
        return -1;
    }

    if (!isInitSuccess) {
        LOGE(TAG, "Error: decoder init error");
        return -2;
    }

    int data_size = av_get_bytes_per_sample(mCodecContext->sample_fmt);
    if (data_size < 0) {
        /* This should not occur, checking just for paranoia */
        LOGE(TAG, "Failed to calculate data size\n");
        return -3;
    }

    int decodeResult = -1;
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
        outFrame->pts = mOutFrame->pts;
        outFrame->dts = outFrame->pts;
//        LOGE(TAG, "data_size = %d, nb_samples = %d, channels = %d, size = %d\n",
//             data_size, mOutFrame->nb_samples, mCodecContext->channels, outFrame->size);
        decodeResult = 0;
    }

    return decodeResult;
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