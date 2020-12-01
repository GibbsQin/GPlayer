//
// Created by qinshenghua on 2020/12/1.
//

#ifndef GPLAYER_DEMUXINGTHREAD_H
#define GPLAYER_DEMUXINGTHREAD_H

#include <thread>
#include "../media/Media.h"

extern "C" {
#include "../protocol/remuxing.h"
}

using namespace std;

class DemuxingThread {
public:
    DemuxingThread(std::function<void(char *, int, FfmpegCallback, MediaInfo *)> func,
                   char *url, int channelId, FfmpegCallback callback, MediaInfo *mediaInfo);

    ~DemuxingThread();

    void join() { mThread->join(); }

private:
    std::thread *mThread = nullptr;
};


#endif //GPLAYER_DEMUXINGTHREAD_H
