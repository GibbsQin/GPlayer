#include "LoopThread.h"

#include <utility>

LoopThread::LoopThread() {
    this->maxValue = MAX_VALUE;
    setFunction(std::bind(&LoopThread::handleRunning, this));
}

LoopThread::LoopThread(int maxSize) {
    this->maxValue = maxSize;
    setFunction(std::bind(&LoopThread::handleRunning, this));
}

LoopThread::~LoopThread()= default;

void LoopThread::handleRunning() {
    int64_t time;

    if (startFunc) {
        startFunc();
    }
    isStarted = true;

    while (mRunning) {
        if (mPausing) {
            std::unique_lock<std::mutex> lck(threadLock);
            conVar.wait(lck);
            continue;
        }
        if (!updateFunc) {
            break;
        }
        int count = updateFunc(arg1, arg2);
        arg1 = -1;
        arg2 = -1;
        if (count > maxValue) {
            sleepTimeMs += SLEEP_TIME_GAP;
        } else {
            sleepTimeMs = 0;
        }
        if (sleepTimeMs > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTimeMs));
        }
    }

    if (endFunc) {
        endFunc();
    }
    isStarted = false;
}

void LoopThread::setStartFunc(std::function<void(void)> func) {
    startFunc = std::move(func);
}

void LoopThread::setUpdateFunc(std::function<int(int, long)> func) {
    updateFunc = std::move(func);
}

void LoopThread::setEndFunc(std::function<void(void)> func) {
    endFunc = std::move(func);
}

void LoopThread::resume() {
    XThread::resume();
    std::unique_lock<std::mutex> lck(threadLock);
    conVar.notify_all();
}

bool LoopThread::hasStarted() {
    return isStarted;
}

void LoopThread::setArgs(int arg1, long arg2) {
    this->arg1 = arg1;
    this->arg2 = arg2;
}
