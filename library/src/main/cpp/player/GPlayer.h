//
// Created by Gibbs on 2020/7/16.
//

#ifndef GPLAYER_GPLAYER_H
#define GPLAYER_GPLAYER_H

#include <codec/DecodeHelper.h>
#include <player/GPlayerJni.h>
#include "LoopThread.h"
#include "InputSource.h"
#include "OutputSource.h"

extern "C" {
#include <demuxing/demuxing.h>
}

#define AV_FLAG_SOURCE_MEDIA_CODEC 0x00000002

#define MAX_BUFFER_SIZE 30

#define MSG_TYPE_ERROR 0
#define MSG_TYPE_STATE 1
#define MSG_TYPE_TIME  2
#define MSG_TYPE_SIZE  3

#define STATE_IDLE      0
#define STATE_PREPARING 1
#define STATE_PREPARED  2
#define STATE_PAUSED    3
#define STATE_PLAYING   4
#define STATE_STOPPING  5
#define STATE_STOPPED   6
#define STATE_RELEASED  7

#define MSG_TYPE_SIZE_AUDIO_PACKET 1
#define MSG_TYPE_SIZE_VIDEO_PACKET 2
#define MSG_TYPE_SIZE_AUDIO_FRAME  3
#define MSG_TYPE_SIZE_VIDEO_FRAME  4

class GPlayer {

public:
    GPlayer(int channelId, uint32_t flag, jobject obj);

    ~GPlayer();

public:
    void av_init(FormatInfo *formatInfo);

    uint32_t av_feed_audio(AVPacket *packet, AVRational time_base);

    uint32_t av_feed_video(AVPacket *packet, AVRational time_base);

    void av_destroy();

    void av_error(int code, char *msg);

public:

    void prepare(const std::string& url);

    void start();

    void pause();

    void resume();

    void seekTo(uint32_t secondUs);

    void stop();

    void setFlags(uint32_t flags);

    LoopFlag loopWait(int64_t *seekUs);

    InputSource *getInputSource();

    OutputSource *getOutputSource();

private:
    int startDemuxing();

    void startDecode();

    void stopDecode();

    int processAudioBuffer();

    int processVideoBuffer();

private:
    InputSource *inputSource;
    OutputSource *outputSource;
    GPlayerJni *playerJni;

private:
    bool mediaCodecFlag;
    char *mUrl{};
    int mChannelId;
    bool isDemuxing{};
    uint32_t mFlags;
    LoopThread *audioDecodeThread;
    LoopThread *videoDecodeThread;
    LoopThread *demuxingThread;
    DecodeHelper *decodeHelper;
    bool mIsPausing;
    int mSeekUs;
};


#endif //GPLAYER_GPLAYER_H
