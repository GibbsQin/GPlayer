#include <base/Log.h>
#include "FfmpegVideoEncoder.h"

FfmpegVideoEncoder::FfmpegVideoEncoder() {
    isInitSuccess = false;
    mCodec = nullptr;
    mCodecContext = nullptr;
    mInFrame = nullptr;
    mOutPacket = nullptr;
    mPts = 0;
}

void FfmpegVideoEncoder::init(MediaInfo *header) {
    mAVHeader = *header;

    av_register_all();

    mCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!mCodec) {
        LOGE(TAG, "Error: encode codec not found");
        return;
    }
    mCodecContext = avcodec_alloc_context3(mCodec);
    if (mCodecContext == nullptr) {
        LOGE(TAG, "Error: context alloc fail");
        return;
    }
    mCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    mCodecContext->codec_id = AV_CODEC_ID_H264;
    mCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    //ffmpeg的CBR可以控制得非常好，与设定值br十分接近
    //ffmpeg的VBR控制得非常不好,高码流max_rate没有限制
    //设置CBR
    mCodecContext->bit_rate = DEFAULT_BIT_RATE;//160000
    mCodecContext->rc_min_rate = DEFAULT_BIT_RATE; //大码流，x264中单位kbps，ffmpeg中单位bps
    mCodecContext->rc_max_rate = DEFAULT_BIT_RATE; //小码流
    mCodecContext->rc_buffer_size = DEFAULT_BIT_RATE;
    mCodecContext->rc_initial_buffer_occupancy = mCodecContext->rc_buffer_size * 3 / 4;
    mCodecContext->width = mAVHeader.videoWidth;
    mCodecContext->height = mAVHeader.videoHeight;
    mCodecContext->time_base = AVRational{1, mAVHeader.videoFrameRate};
    mCodecContext->gop_size = 48;
    mCodecContext->max_b_frames = 0;
    //avc->refs = 2;
    //B和P帧向前预测参考的帧数，取值范围-16，该值不影响解码的速度，但越大解码内存越大,一般越大效果越大
    mCodecContext->b_frame_strategy = true;
    mCodecContext->keyint_min = 8;
    mCodecContext->i_quant_factor = 0.71;
    mCodecContext->trellis = 1;
    mCodecContext->me_cmp = FF_CMP_CHROMA;//码率失真最优的宏块模式
    // zero phods log x1 hex umh epzs(默认) full(完全搜索，很慢又好不到哪里去)
    mCodecContext->me_range = 16;
    mCodecContext->max_qdiff = 4;//指定固定量化器因子允许的最大偏移
    mCodecContext->qmax = 30;
    mCodecContext->qmin = 20;
    mCodecContext->qblur = 0.6; //指定量化器模糊系数，可用范围0.0-1.0越大使得码率在时间上分配的越平均
    mCodecContext->qcompress = 0.7;//指定视频量化器压缩系数，默认0.5
    mCodecContext->me_subpel_quality = 1;//这个参数控制在运动估算过程中质量和速度的权衡Subq=5可以压缩>10%于subq=1 -7
    mCodecContext->compression_level = 1;
    mCodecContext->thread_count = 4;
    mCodecContext->level = 12;
    //av_opt_set(mCodecContext->priv_data, "preset", "fast", 0);
    av_opt_set(mCodecContext->priv_data, "preset", "ultrafast", 0);
    av_opt_set(mCodecContext->priv_data, "tune", "zerolatency", 0);
    av_opt_set(mCodecContext->priv_data, "x264opts", "no-mbtree:sliced-threads:sync-lookahead=0", 0);
    //av_dict_set(&opt, "b", "4.0M", 0);
    char strSize[20] = {0};
    sprintf(strSize, "%dx%d", header->videoWidth, header->videoHeight);
    AVDictionary *opt = nullptr;
    av_dict_set(&opt, "video_size", strSize, 0);
    av_dict_set(&opt, "profile", "baseline", 0);
    if (avcodec_open2(mCodecContext, mCodec, &opt) < 0) {
        LOGE(TAG, "Error: could not open encode-codec");
        return;
    }

    mOutPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    av_init_packet(mOutPacket);

    mInFrame = av_frame_alloc();
    if (mInFrame == nullptr) {
        LOGE(TAG, "Error: av_frame_alloc fail");
        return;
    }

    isInitSuccess = true;
}

int FfmpegVideoEncoder::send_frame(MediaData *inFrame) {
    if (inFrame == nullptr) {
        LOGE(TAG, "Error: the param is nullptr");
        return -1;
    }

    if (!isInitSuccess) {
        LOGE(TAG, "Error: init error");
        return -2;
    }

    //填充数据
    mInFrame->format = mCodecContext->pix_fmt;
    mInFrame->data[0] = inFrame->data;
    mInFrame->data[1] = inFrame->data1;
    mInFrame->data[2] = inFrame->data2;
    mInFrame->linesize[0] = inFrame->width;
    mInFrame->linesize[1] = inFrame->width / 2;
    mInFrame->linesize[2] = inFrame->width / 2;
    mInFrame->width = inFrame->width;
    mInFrame->height = inFrame->height;
    mInFrame->pts = mPts;
    mPts += 1;

    int ret = avcodec_send_frame(mCodecContext, mInFrame);
    if (ret < 0) {
        LOGE(TAG, "Error: avcodec_send_frame %d %s", ret, av_err2str(ret));
        return ret;
    }

    return 0;
}

int FfmpegVideoEncoder::receive_packet(MediaData *outPacket) {
    int ret = avcodec_receive_packet(mCodecContext, mOutPacket);
    if (ret < 0) {
        LOGE(TAG, "Error: avcodec_receive_packet %d %s", ret, av_err2str(ret));
        return ret;
    }

    memcpy(outPacket->data, mOutPacket->data, mOutPacket->size);
    outPacket->size = mOutPacket->size;
    outPacket->pts = mOutPacket->pts;
    outPacket->dts = mOutPacket->dts;
    outPacket->flag = mInFrame->key_frame;

    av_packet_unref(mOutPacket);

    return 0;
}

void FfmpegVideoEncoder::release() {
    if (mOutPacket != nullptr) {
        av_packet_unref(mOutPacket);
        av_free(mOutPacket);
        mOutPacket = nullptr;
    }

    if (mInFrame != nullptr) {
        av_frame_free(&mInFrame);
        mInFrame = nullptr;
    }
    if (mCodecContext != nullptr) {
        avcodec_close(mCodecContext);
        avcodec_free_context(&mCodecContext);
        mCodecContext = nullptr;
    }
}

