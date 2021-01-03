/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#ifndef GPLAYER_GPLAYER_H
#define GPLAYER_GPLAYER_H

#include <android/native_window.h>
#include "DecoderHelper.h"
#include "DemuxerHelper.h"
#include "MessageHelper.h"
#include "GPlayerJni.h"
#include "LoopThread.h"
#include "PacketSource.h"
#include "FrameSource.h"
#include "MessageSource.h"
#include "RenderHelper.h"

#define AV_FLAG_SOURCE_MEDIA_CODEC 0x00000002

class GPlayer {

public:
    GPlayer(uint32_t flag, jobject obj);

    ~GPlayer();

public:
    void setSurface(ANativeWindow *window);

    void setAudioTrack(AudioTrackJni *track);

    void prepare(const std::string &url);

    void start();

    void pause();

    void resume();

    void seekTo(uint32_t secondUs);

    void stop();

    void setFlags(uint32_t flags) {
        mFlags = flags;
    }

    int getDuration();

    int getVideoWidth();

    int getVideoHeight();

    void onDemuxingChanged(int state);

    void onDecodingChanged(int state);

    void onRenderingChanged(int state);

    void onSeekStateChanged(int state);

    void onBufferStateChanged(int state);

    void onPlayStateChanged(int state);

private:
    int processMessage(int arg1, long arg2);

    void startMessageLoop();

    void stopMessageLoop();

    void startDemuxing(const std::string &url);

    void stopDemuxing();

    void startDecoding();

    void stopDecoding();

    void startRendering();

    void stopRendering();

    void pauseThreads(bool demuxing, bool decoding, bool rendering, bool messaging);

    void resumeThreads(bool demuxing, bool decoding, bool rendering, bool messaging);

private:
    uint32_t mFlags;
    ANativeWindow *nativeWindow = nullptr;
    AudioTrackJni *audioTrackJni = nullptr;

    PacketSource *packetSource;
    FrameSource *frameSource;
    MessageSource *messageSource;
    DemuxerHelper *demuxerHelper;
    DecoderHelper *decoderHelper;
    RenderHelper  *renderHelper;
    MessageHelper *messageHelper;

    LoopThread *demuxerThread;
    LoopThread *audioDecodeThread;
    LoopThread *videoDecodeThread;
    LoopThread *audioRenderThread;
    LoopThread *videoRenderThread;
    LoopThread *messageThread;

    int playState = -1;
    int bufferState = -1;
    int seekState = -1;
    bool isEof = false;
    std::mutex playerLock;
};


#endif //GPLAYER_GPLAYER_H
