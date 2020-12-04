#ifndef GPLAYER_CODEC_H
#define GPLAYER_CODEC_H

#include "media/MediaData.h"
extern "C" {
#include <demuxing/avformat_def.h>
}

#define FLAG_KEY_FRAME 0x00000001
#define FLAG_KEY_EXTRA_DATA 0x00000002
#define FLAG_KEY_RENDERED 0x00000004

#define TRY_AGAIN -11

/**
 * 音频编码器
 */
class AudioEncoder {
public:
    virtual ~AudioEncoder() {};

    /**
    * 初始化
    */
    virtual void init(MediaInfo *header) = 0;

    /**
     * 往编码器输入一帧
     * @param inFrame
     * @return 结果
     */
    virtual int send_frame(MediaData *inFrame) = 0;

    /**
     * 从编码器获取解码后的一帧
     * @param outPacket
     * @return 结果
     */
    virtual int receive_packet(MediaData *outPacket) = 0;

    /**
     * 释放
     */
    virtual void release() = 0;
};

/**
 * 音频解码器
 */
class AudioDecoder {
public:
    virtual ~AudioDecoder() {};

    /**
     * 初始化
     */
    virtual void init(MediaInfo *header) = 0;

    /**
     * 往解码器输入一帧
     * @param inPacket
     * @return 结果
     */
    virtual int send_packet(MediaData *inPacket) = 0;

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
};

/**
 * 视频编码器
 */
class VideoEncoder {
public:
    virtual ~VideoEncoder() {};

    /**
     * 初始化
     */
    virtual void init(MediaInfo *header) = 0;

    /**
     * 往编码器输入一帧
     * @param inFrame
     * @return 结果
     */
    virtual int send_frame(MediaData *inFrame) = 0;

    /**
     * 从编码器获取解码后的一帧
     * @param outPacket
     * @return 结果
     */
    virtual int receive_packet(MediaData *outPacket) = 0;

    /**
     * 释放
     */
    virtual void release() = 0;
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
    virtual void init(MediaInfo *header) = 0;

    /**
     * 往解码器输入一帧
     * @param inPacket
     * @return 结果
     */
    virtual int send_packet(MediaData *inPacket) = 0;

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
};

#endif //GPLAYER_CODEC_H
