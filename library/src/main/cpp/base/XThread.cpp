﻿#include "XThread.h"
#include "Log.h"

#define TAG "Thread"

XThread::XThread() {
}

XThread::~XThread() {
}

bool XThread::start() {
    if (mRunning){
        return false;
    }

    if(!mFunc){
        LOGE("XThread", "func is not init");
        return false;
    }

    delete mThread;
    mRunning = true;
    mThread = new std::thread(mFunc);

    return true;
}

void XThread::pause() {
    mPausing = true;
}

void XThread::resume() {
    mPausing = false;
}

bool XThread::stop() {
    if (!mRunning) {
        return true;
    }

    mRunning = false;

    return true;
}

void XThread::join() {
    if (mThread != nullptr && mThread->joinable()) {
        mThread->join();
    }
}
