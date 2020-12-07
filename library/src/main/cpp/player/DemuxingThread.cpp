//
// Created by qinshenghua on 2020/12/1.
//

#include "DemuxingThread.h"

DemuxingThread::DemuxingThread(std::function<void(char *, int, FfmpegCallback, FormatInfo *)> func,
                               char *url, int channelId, FfmpegCallback callback, FormatInfo *formatInfo) {
    mThread = new std::thread(func, url, channelId, callback, formatInfo);
}

DemuxingThread::~DemuxingThread() {
    if (mThread) {
        if (mThread->joinable()) {
            mThread->join();
        }
        mThread = nullptr;
    }
}
