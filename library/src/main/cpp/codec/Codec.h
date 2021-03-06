/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#ifndef GPLAYER_CODEC_H
#define GPLAYER_CODEC_H

#include "media/MediaData.h"
#include <android/native_window.h>

extern "C" {
#include <demuxing/avformat_def.h>
}

#define TRY_AGAIN      -11
#define INVALID_CODEC  -12

/**
 * 音频解码器
 */
class AudioDecoder {
public:
    virtual ~AudioDecoder() {};

    /**
     * 初始化
     */
    virtual void init(AVCodecParameters *codecParameters) = 0;

    /**
     * 往解码器输入一帧
     * @param inPacket
     * @return 结果
     */
    virtual int send_packet(AVPacket *inPacket) = 0;

    /**
     * 从解码器获取解码后的一帧
     * @param outFrame
     * @return 结果
     */
    virtual int receive_frame(MediaData *outFrame) = 0;

    /**
     * 释放
     */
    virtual void release() = 0;

    /**
     * 重置
     */
    virtual void reset() = 0;
};

/**
 * 视频解码器
 */
class VideoDecoder {
public:
    virtual ~VideoDecoder() {};

    /**
     * 初始化
     */
    virtual void init(AVCodecParameters *codecParameters) = 0;

    /**
     * 往解码器输入一帧
     * @param inPacket
     * @return 结果
     */
    virtual int send_packet(AVPacket *inPacket) = 0;

    /**
     * 从解码器获取解码后的一帧
     * @param outFrame
     * @return 结果
     */
    virtual int receive_frame(MediaData *outFrame) = 0;

    /**
     * 用于MediaCodec渲染一帧
     */
    virtual void release_buffer() = 0;

    /**
     * 释放
     */
    virtual void release() = 0;

    /**
     * 重置
     */
    virtual void reset() = 0;

    /**
     * 设置nativeWindow
     * @param nativeWindow
     */
    virtual void setNativeWindow(ANativeWindow *nativeWindow) = 0;
};

#endif //GPLAYER_CODEC_H
