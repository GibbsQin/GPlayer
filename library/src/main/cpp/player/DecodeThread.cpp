#include "DecodeThread.h"

#include <utility>

DecodeThread::DecodeThread() {
    setFunction(std::bind(&DecodeThread::handleRunning, this));
}

DecodeThread::~DecodeThread()= default;

void DecodeThread::handleRunning() {
    int64_t time;

    if (startFunc) {
        startFunc();
    }
    isStarted = true;

    while (mRunning) {
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

void DecodeThread::setStartFunc(std::function<void(void)> func) {
    startFunc = std::move(func);
}

void DecodeThread::setUpdateFunc(std::function<int(void)> func) {
    updateFunc = std::move(func);
}

void DecodeThread::setEndFunc(std::function<void(void)> func) {
    endFunc = std::move(func);
}

bool DecodeThread::hasStarted() {
    return isStarted;
}
