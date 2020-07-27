package com.gibbs.gplayer.codec;

import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.media.MediaInfo;

interface BaseEncoder {
    /**
     * 初始化
     *
     * @param header 音频编码参数
     */
    void init(MediaInfo header);

    /**
     * 往编码器输入一帧
     *
     * @param inFrame 音频帧原始数据
     * @return >=0 成功，其他值 失败
     */
    int send_frame(MediaData inFrame);

    /**
     * 从编码器获取解码后的一帧
     *
     * @param outPacket 音频编码后的数据（空间大小需要自己分配）
     * @return >=0 成功，其他值 失败
     */
    int receive_packet(MediaData outPacket);

    /**
     * 释放资源
     */
    void release();
}
