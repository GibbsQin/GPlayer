#include "LoopThread.h"

#include <utility>

LoopThread::LoopThread() {
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
            std::this_thread::sleep_for(std::chrono::milliseconds(MAX_SLEEP_TIME));
            continue;
        }
        if (!updateFunc) {
            break;
        }
        int count = updateFunc();
        if (count > MAX_OUTPUT_FRAME_SIZE) {
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

void LoopThread::setUpdateFunc(std::function<int(void)> func) {
    updateFunc = std::move(func);
}

void LoopThread::setEndFunc(std::function<void(void)> func) {
    endFunc = std::move(func);
}

bool LoopThread::hasStarted() {
    return isStarted;
}
