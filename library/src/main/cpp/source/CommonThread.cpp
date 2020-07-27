#include "CommonThread.h"

#include <utility>
#include "../base/XTick.h"

CommonThread::CommonThread(int fps) {
    this->fps = fps;
    frameInterval = (int64_t) (1.0f / (float) fps * 1000);
    currentTime = getTickCount64();
    setFunction(std::bind(&CommonThread::handleRunning, this));
}

CommonThread::~CommonThread()= default;

void CommonThread::handleRunning() {
    int64_t time;

    if (startFunc) {
        startFunc();
    }
    isStarted = true;

    while (mRunning) {
        time = getTickCount64();
        if (!updateFunc) {
            break;
        }
        updateFunc(time);
        currentTime = getTickCount64();
        time = frameInterval - (currentTime - time);
        time = time < 0 ? 1 : time;
        time = time > frameInterval ? frameInterval : time;
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }

    if (endFunc) {
        endFunc();
    }
    isStarted = false;
}

void CommonThread::setStartFunc(std::function<void(void)> func) {
    startFunc = std::move(func);
}

void CommonThread::setUpdateFunc(std::function<void(int64_t)> func) {
    updateFunc = std::move(func);
}

void CommonThread::setEndFunc(std::function<void(void)> func) {
    endFunc = std::move(func);
}

bool CommonThread::hasStarted() {
    return isStarted;
}
