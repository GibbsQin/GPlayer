//
// Created by qinshenghua on 2020/12/1.
//

#include "DemuxingThread.h"

DemuxingThread::DemuxingThread(std::function<void(char*, int, FfmpegCallback, MediaInfo*)> func,
                               char* url, int channelId, FfmpegCallback callback, MediaInfo* mediaInfo) {
    mThread = new std::thread(func, url, channelId, callback, mediaInfo);
}

DemuxingThread::~DemuxingThread() {
    if (mThread) {
        mThread->join();
        mThread = nullptr;
    }
}
