
#include "base/Log.h"
#include "FfmpegAudioDecoder.h"

FfmpegAudioDecoder::FfmpegAudioDecoder() {
    isInitSuccess = false;
    mCodec = nullptr;
    mCodecContext = nullptr;
#ifdef ENABLE_PARSER
    mParser = nullptr;
#endif
    mInPacket = nullptr;
    mOutFrame = nullptr;
}

FfmpegAudioDecoder::~FfmpegAudioDecoder() = default;

void FfmpegAudioDecoder::init(AVCodecParameters *codecParameters) {
    av_register_all();

#ifdef ENABLE_PARSER
    mParser = av_parser_init(mCodec->id);
#endif
    mCodecContext = avcodec_alloc_context3(mCodec);
    avcodec_parameters_from_context(codecParameters, mCodecContext);
    mCodec = avcodec_find_decoder(mCodecContext->codec_id);

    LOGI(TAG, "init sample_fmt=%d,sample_rate=%d,channel_layout=%lld,channels=%d,frame_size=%d",
         mCodecContext->sample_fmt, mCodecContext->sample_rate, mCodecContext->channel_layout,
         mCodecContext->channels, mCodecContext->frame_size);

    int ret = avcodec_open2(mCodecContext, mCodec, nullptr);
    if (ret < 0) {
        LOGE(TAG, "Error: avcodec_open2 code is %d", ret);
        return;
    }

    mInPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    av_init_packet(mInPacket);
    mOutFrame = av_frame_alloc();
    isInitSuccess = true;

#ifdef SAVE_DECODE_FILE
    audioFile = fopen("/sdcard/Android/data/com.gibbs.gplayer/files/Movies/decode_audio.pcm", "wb+");
#endif
}

int FfmpegAudioDecoder::send_packet(MediaData *inPacket) {
    if (inPacket == nullptr) {
        LOGE(TAG, "Error: decode the param is nullptr");
        return -1;
    }

    if (!isInitSuccess) {
        LOGE(TAG, "Error: decoder init error");
        return -2;
    }

    int ret;
#ifdef ENABLE_PARSER
    av_init_packet(mInPacket);
    ret = av_parser_parse2(mParser, mCodecContext, &mInPacket->data, &mInPacket->size,
                               inPacket->data, inPacket->size,
                               AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
    if (ret < 0) {
        LOGE(TAG, "Error while parsing\n");
        return -1;
    }
#else
    mInPacket->data = inPacket->data;
    mInPacket->size = inPacket->size;
#endif
    mInPacket->pts = inPacket->pts;
    mInPacket->dts = inPacket->dts;

    ret = avcodec_send_packet(mCodecContext, mInPacket);
    if (ret < 0) {
        LOGE(TAG, "Error: avcodec_send_packet %d %s", ret, av_err2str(ret));
        return ret;
    }

#ifdef ENABLE_PARSER
    av_packet_unref(mInPacket);
#endif

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

#ifdef SAVE_DECODE_FILE
    if (audioFile && outFrame->size > 0) {
        fwrite(outFrame->data, 1, outFrame->size, audioFile);
    }
#endif

    return decodeResult;
}

void FfmpegAudioDecoder::release() {
    LOGI(TAG, "release");
    if (mInPacket != nullptr) {
        av_packet_unref(mInPacket);
        mInPacket = nullptr;
    }

    if (mOutFrame != nullptr) {
        av_frame_free(&mOutFrame);
        mOutFrame = nullptr;
    }

    if (mCodecContext != nullptr) {
        avcodec_close(mCodecContext);
        avcodec_free_context(&mCodecContext);
        mCodecContext = nullptr;
    }

#ifdef ENABLE_PARSER
    if (mParser != nullptr) {
        av_parser_close(mParser);
        mParser = nullptr;
    }
#endif

#ifdef SAVE_DECODE_FILE
    if (audioFile) {
        fclose(audioFile);
        audioFile = nullptr;
    }
#endif
}