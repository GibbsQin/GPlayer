#ifndef GPLAYER_MEDIASOURCE_H
#define GPLAYER_MEDIASOURCE_H


#include <jni.h>
#include <string>
#include <queue>
#include "Codec.h"
#include "CommonThread.h"

#define AV_SOURCE_RELEASE -1
#define AV_SOURCE_EMPTY -2

#define AV_FLAG_SOURCE_DECODE 0x00000001
#define AV_FLAG_SOURCE_MEDIA_CODEC 0x00000002

class MediaSource {

public:

    MediaSource();

    ~MediaSource();

    int getChannelId();

    void setChannelId(int ch);

    /**
     * 初始化
     *
     * @param header 音视频参数
     */
    void onInit(MediaInfo *header);

    /**
     * 接收到一帧音频
     *
     * @param inPacket 音频帧数据
     */

    uint32_t onReceiveAudio(MediaData *inPacket);

    /**
     * 接收到一帧视频
     *
     * @param inPacket 音频帧数据
     */
    uint32_t onReceiveVideo(MediaData *inPacket);

    /**
     * 释放资源
     */
    void onRelease();

    void flushBuffer();

    void flushAudioBuffer();

    void flushVideoBuffer();

    void pendingFlushBuffer();

    MediaInfo* getAVHeader();

    int readAudioBuffer(MediaData** avData);

    int readVideoBuffer(MediaData** avData);

    void popAudioBuffer();

    void popVideoBuffer();

private:
    MediaInfo *mAVHeader{};
    bool isRelease{};
    int channelId{};
    std::queue<MediaData *> videoPacketQueue;
    std::queue<MediaData *> audioPacketQueue;
    std::mutex mAudioLock;
    std::mutex mVideoLock;
    bool isPendingFlushBuffer;
};


#endif //GPLAYER_MEDIASOURCE_H
