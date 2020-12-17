//
// Created by Gibbs on 2020/7/16.
//

#ifndef GPLAYER_GPLAYER_H
#define GPLAYER_GPLAYER_H

#include <player/DecoderHelper.h>
#include <player/GPlayerJni.h>
#include <player/DemuxerHelper.h>
#include "LoopThread.h"
#include "PacketSource.h"
#include "FrameSource.h"
#include "MessageQueue.h"

#define AV_FLAG_SOURCE_MEDIA_CODEC 0x00000002

#define MAX_BUFFER_SIZE 30

class GPlayer {

public:
    GPlayer(uint32_t flag, jobject obj);

    ~GPlayer();

public:

    void prepare(const std::string& url);

    void start();

    void pause();

    void resume();

    void seekTo(uint32_t secondUs);

    void stop();

    void setFlags(uint32_t flags);

    PacketSource *getInputSource();

    FrameSource *getOutputSource();

private:
    void startMessageLoop();

    void stopMessageLoop();

    int processMessage(int arg1, long arg2);

    void startDemuxing(const std::string &url);

    void stopDemuxing();

    void startDecode();

    void stopDecode();

private:
    PacketSource *inputSource;
    FrameSource *outputSource;
    GPlayerJni *playerJni;

private:
    uint32_t mFlags;
    LoopThread *audioDecodeThread;
    LoopThread *videoDecodeThread;
    LoopThread *demuxerThread;
    LoopThread *messageThread;
    DemuxerHelper *demuxerHelper;
    DecoderHelper *decoderHelper;
    MessageQueue *messageQueue;
    int mSeekUs;
};


#endif //GPLAYER_GPLAYER_H
