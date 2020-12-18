//
// Created by Gibbs on 2020/7/16.
//

#ifndef GPLAYER_GPLAYER_H
#define GPLAYER_GPLAYER_H

#include "DecoderHelper.h"
#include "DemuxerHelper.h"
#include "MessageHelper.h"
#include "GPlayerJni.h"
#include "LoopThread.h"
#include "PacketSource.h"
#include "FrameSource.h"
#include "MessageQueue.h"

#define AV_FLAG_SOURCE_MEDIA_CODEC 0x00000002

#define MAX_BUFFER_PACKET_SIZE 30
#define MAX_BUFFER_FRAME_SIZE  5

class GPlayer {

public:
    GPlayer(uint32_t flag, jobject obj);

    ~GPlayer();

public:
    void prepare(const std::string &url);

    void start();

    void pause();

    void resume();

    void seekTo(uint32_t secondUs);

    void stop();

public:
    void setFlags(uint32_t flags);

    PacketSource *getInputSource();

    FrameSource *getOutputSource();

private:
    void startMessageLoop();

    void stopMessageLoop();

    void startDemuxing(const std::string &url);

    void stopDemuxing();

    void startDecode();

    void stopDecode();

private:
    uint32_t mFlags;

    PacketSource *inputSource;
    FrameSource *outputSource;
    MessageQueue *messageQueue;
    DemuxerHelper *demuxerHelper;
    DecoderHelper *decoderHelper;
    MessageHelper *messageHelper;

    LoopThread *audioDecodeThread;
    LoopThread *videoDecodeThread;
    LoopThread *demuxerThread;
    LoopThread *messageThread;
};


#endif //GPLAYER_GPLAYER_H
