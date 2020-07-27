#include "base/Log.h"
#include "FfmpegAudioEncoder.h"

FfmpegAudioEncoder::FfmpegAudioEncoder() {
    isInitSuccess = false;
    mCodec = nullptr;
    mCodecContext = nullptr;
    mOrgAvFrame = nullptr;
    mSwrFrame = nullptr;
    mOutPacket = nullptr;
    mSwrCtx = nullptr;
    mPts = 0;
}

void FfmpegAudioEncoder::init(MediaInfo *header) {
    mHeader = *header;
    av_register_all();
    mCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    mIsNeedSwr = true;//ffmpeg自带的aac编译器只支持AV_SAMPLE_FMT_FLTP，但是录音是AV_SAMPLE_FMT_S16，所以需要转码
    if (!mCodec) {
        LOGE(TAG, "Error: encode codec not found");
        return;
    }

    mCodecContext = avcodec_alloc_context3(mCodec);
    if (mCodecContext == nullptr) {
        LOGE(TAG, "Error: context alloc fail");
        return;
    }

    mOriginAVSampleFormat = AV_SAMPLE_FMT_S16;
    mCodecContext->codec_id = AV_CODEC_ID_AAC;
    mCodecContext->codec_type = AVMEDIA_TYPE_AUDIO;
    mCodecContext->sample_fmt = (AVSampleFormat)mHeader.audioBitWidth;
    mCodecContext->sample_rate = mHeader.audioSampleRate; //音频采样率
    mCodecContext->channel_layout = static_cast<uint64_t>(mHeader.audioMode);//声道格式（单声道、双声道）
    mCodecContext->channels = av_get_channel_layout_nb_channels(mCodecContext->channel_layout);//声道数
    mCodecContext->frame_size = mHeader.sampleNumPerFrame;
    mCodecContext->bit_rate = 64000;            //平均码率
    mCodecContext->profile = FF_PROFILE_AAC_LOW;
    //mCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    int ret = avcodec_open2(mCodecContext, mCodec, nullptr);
    if (ret < 0) {
        LOGE(TAG, "Error: avcodec_open2 code is %d", ret);
        return;
    }
    LOGD(TAG, "header sampleNum is %d, frame_size is %d, channel is %d, sample_fmt is %d",
         mHeader.sampleNumPerFrame, mCodecContext->frame_size, mCodecContext->channels, mCodecContext->sample_fmt);

    if (mIsNeedSwr) {
        mHeader.audioBitWidth = mCodecContext->sample_fmt;
        header->audioBitWidth = mHeader.audioBitWidth;
    }

    mOutPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    av_init_packet(mOutPacket);

    mOrgAvFrame = av_frame_alloc();
    mOrgAvFrame->format = mOriginAVSampleFormat;
    mOrgAvFrame->nb_samples = mHeader.sampleNumPerFrame;
    mOrgAvFrame->channels = mCodecContext->channels;
    mOrgAvFrame->channel_layout = mCodecContext->channel_layout;
    mOrgAvFrame->sample_rate = mCodecContext->sample_rate;

    mSwrFrame = av_frame_alloc();
    mSwrFrame->format = mCodecContext->sample_fmt;
    mSwrFrame->nb_samples = mOrgAvFrame->nb_samples;
    mSwrFrame->channels = mOrgAvFrame->channels;
    mSwrFrame->channel_layout = mOrgAvFrame->channel_layout;
    mSwrFrame->sample_rate = mOrgAvFrame->sample_rate;

    /*************************************************************************/
    //重采样设置参数
    /*************************************************************************/
    mSwrCtx = swr_alloc();
    swr_alloc_set_opts(mSwrCtx,
                       mCodecContext->channel_layout, mCodecContext->sample_fmt, mCodecContext->sample_rate,
                       mCodecContext->channel_layout, mOriginAVSampleFormat,     mCodecContext->sample_rate,
                       0, nullptr);
    ret = swr_init(mSwrCtx);
    if (ret < 0) {
        LOGE(TAG, "Error: swr_init error, code is %d", ret);
        return;
    }

#ifdef ADD_ADTS_TO_AVPACKET
    //init adts context
    ret = aac_decode_extradata(&mADTSContext, mCodecContext->extradata, mCodecContext->extradata_size);
    if (ret < 0) {
        LOGE(TAG, "Error: fail to init adts context, code is %d", ret);
    }
    LOGI(TAG, "mADTSContext write_adts=%d, objecttype=%d, channel_conf=%d, sample_rate_index=%d",
            mADTSContext.write_adts, mADTSContext.objecttype, mADTSContext.channel_conf, mADTSContext.sample_rate_index);
#endif

    isInitSuccess = true;
    mPts = 0;
}

int FfmpegAudioEncoder::send_frame(MediaData *inFrame) {
    if (inFrame == nullptr) {
        LOGE(TAG, "Error: encode the param is nullptr");
        return -1;
    }

    if (!isInitSuccess) {
        LOGE(TAG, "Error: encode init error");
        return -2;
    }

    int ret = avcodec_fill_audio_frame(mOrgAvFrame, mCodecContext->channels, mOriginAVSampleFormat,
                                       (const uint8_t *) inFrame->data, inFrame->size, 1);
    if (ret < 0) {
        LOGE(TAG, "Error: avcodec_fill_audio_frame code is %d, %s", ret, av_err2str(ret));
        return ret;
    }

    if (mIsNeedSwr) {
        ret = swr_convert_frame(mSwrCtx, mSwrFrame, mOrgAvFrame);
        if (ret < 0) {
            LOGE(TAG, "Error: swr_convert code is %d", ret);
            return ret;
        }
    }

    AVRational av = AVRational{1, mCodecContext->sample_rate};
    mPts += av_rescale_q(mOrgAvFrame->nb_samples, av, mCodecContext->time_base);

    mOrgAvFrame->pts = mPts;
    mSwrFrame->pts = mPts;

    if (mIsNeedSwr) {
        ret = avcodec_send_frame(mCodecContext, mSwrFrame);
    } else {
        ret = avcodec_send_frame(mCodecContext, mOrgAvFrame);
    }

    if (ret < 0) {
        LOGE(TAG, "Error: avcodec_send_frame code is %d, %s", ret, av_err2str(ret));
        return ret;
    }

    return 0;
}

int FfmpegAudioEncoder::receive_packet(MediaData *outPacket) {
    if (!isInitSuccess) {
        LOGE(TAG, "Error: encode init error");
        return -2;
    }
    int ret = avcodec_receive_packet(mCodecContext, mOutPacket);
    if (ret != 0) {
        LOGE(TAG, "Error: avcodec_receive_packet code is %d, %s", ret, av_err2str(ret));
        return ret;
    }
    mOutPacket->pts = av_rescale_q(mOutPacket->pts, mCodecContext->time_base, mCodecContext->time_base);
    mOutPacket->dts = av_rescale_q(mOutPacket->dts, mCodecContext->time_base, mCodecContext->time_base);
    mOutPacket->duration = av_rescale_q(mOutPacket->duration, mCodecContext->time_base, mCodecContext->time_base);

#ifdef ADD_ADTS_TO_AVPACKET
    //add adts header
    aac_set_adts_head(&mADTSContext, mADTSHeader, mOutPacket->size);
    memcpy(outPacket->data, mADTSHeader, ADTS_HEADER_SIZE);
    memcpy(outPacket->data + ADTS_HEADER_SIZE, mOutPacket->data, static_cast<size_t>(mOutPacket->size));
    outPacket->size = static_cast<uint32_t>(mOutPacket->size + ADTS_HEADER_SIZE);
#else
    memcpy(outPacket->data, mOutPacket->data, mOutPacket->size);
    outPacket->size = mOutPacket->size;
#endif
    outPacket->pts = static_cast<uint64_t>(mOutPacket->pts);
    outPacket->dts = static_cast<uint64_t>(mOutPacket->dts);

    av_packet_unref(mOutPacket);

    return 0;
}

void FfmpegAudioEncoder::release() {
    if (mOutPacket) {
        av_packet_unref(mOutPacket);
        av_free(mOutPacket);
        mOutPacket = nullptr;
    }

    if (mOrgAvFrame) {
        av_frame_free(&mOrgAvFrame);
        mOrgAvFrame = nullptr;
    }

    if (mSwrFrame) {
        av_frame_free(&mSwrFrame);
        mSwrFrame = nullptr;
    }

    if (mCodecContext) {
        avcodec_close(mCodecContext);
        avcodec_free_context(&mCodecContext);
        mCodecContext = nullptr;
    }

    if (mSwrCtx) {
        swr_free(&mSwrCtx);
        mSwrCtx = nullptr;
    }
}

int FfmpegAudioEncoder::aac_decode_extradata(ADTSContext *adts, unsigned char *pbuf, int bufsize) {
    int aot, aotext, samfreindex;
    int i, channelconfig;
    unsigned char *p = pbuf;
    if (!adts || !pbuf || bufsize < 2) {
        return -1;
    }
    aot = (p[0] >> 3) & 0x1f;
    if (aot == 31) {
        aotext = (p[0] << 3 | (p[1] >> 5)) & 0x3f;
        aot = 32 + aotext;
        samfreindex = (p[1] >> 1) & 0x0f;
        if (samfreindex == 0x0f) {
            channelconfig = ((p[4] << 3) | (p[5] >> 5)) & 0x0f;
        } else {
            channelconfig = ((p[1] << 3) | (p[2] >> 5)) & 0x0f;
        }
    } else {
        samfreindex = ((p[0] << 1) | p[1] >> 7) & 0x0f;
        if (samfreindex == 0x0f) {
            channelconfig = (p[4] >> 3) & 0x0f;
        } else {
            channelconfig = (p[1] >> 3) & 0x0f;
        }
    }
#ifdef AOT_PROFILE_CTRL
    if (aot < 2) aot = 2;
#endif
    adts->objecttype = aot - 1;
    adts->sample_rate_index = samfreindex;
    adts->channel_conf = channelconfig;
    adts->write_adts = 1;
    return 0;
}

int FfmpegAudioEncoder::aac_set_adts_head(ADTSContext *acfg, unsigned char *buf, int size) {
    unsigned char byte;
    if (size < ADTS_HEADER_SIZE) {
        return -1;
    }
    buf[0] = 0xff;
    buf[1] = 0xf1;
    byte = 0;
    byte |= (acfg->objecttype & 0x03) << 6;
    byte |= (acfg->sample_rate_index & 0x0f) << 2;
    byte |= (acfg->channel_conf & 0x07) >> 2;
    buf[2] = byte;
    byte = 0;
    byte |= (acfg->channel_conf & 0x07) << 6;
    byte |= (ADTS_HEADER_SIZE + size) >> 11;
    buf[3] = byte;
    byte = 0;
    byte |= (ADTS_HEADER_SIZE + size) >> 3;
    buf[4] = byte;
    byte = 0;
    byte |= ((ADTS_HEADER_SIZE + size) & 0x7) << 5;
    byte |= (0x7ff >> 6) & 0x1f;
    buf[5] = byte;
    byte = 0;
    byte |= (0x7ff & 0x3f) << 2;
    buf[6] = byte;
    return 0;
}
