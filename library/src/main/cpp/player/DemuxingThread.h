//
// Created by qinshenghua on 2020/12/1.
//

#ifndef GPLAYER_DEMUXINGTHREAD_H
#define GPLAYER_DEMUXINGTHREAD_H

#include <thread>

extern "C" {
#include "demuxing/demuxing.h"
#include "demuxing/avformat_def.h"
}

using namespace std;

class DemuxingThread {
public:
    DemuxingThread(std::function<void(char *, int, FfmpegCallback, FormatInfo *)> func,
                   char *url, int channelId, FfmpegCallback callback, FormatInfo *formatInfo);

    ~DemuxingThread();

    void join() { mThread->join(); }

private:
    std::thread *mThread = nullptr;
};


#endif //GPLAYER_DEMUXINGTHREAD_H
