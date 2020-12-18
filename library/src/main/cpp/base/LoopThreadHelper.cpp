//
// Created by qinshenghua on 2020/12/18.
//

#include "LoopThreadHelper.h"

LoopThread *LoopThreadHelper::createLoopThread(std::function<int(int, long)> updateFunc) {
    return LoopThreadHelper::createLoopThread(MAX_VALUE, updateFunc);;
}

LoopThread *
LoopThreadHelper::createLoopThread(int maxValue, std::function<int(int, long)> updateFunc) {
    return LoopThreadHelper::createLoopThread(maxValue, nullptr, updateFunc, nullptr);
}

LoopThread *LoopThreadHelper::createLoopThread(int maxValue,
                                               std::function<void(void)> startFunc,
                                               std::function<int(int, long)> updateFunc,
                                               std::function<void()> endFunc) {
    LoopThread *thread = new LoopThread(maxValue);
    if (startFunc != nullptr) {
        thread->setStartFunc(startFunc);
    }
    if (updateFunc != nullptr) {
        thread->setUpdateFunc(updateFunc);
    }
    if (endFunc != nullptr) {
        thread->setEndFunc(endFunc);
    }
    thread->start();
    return thread;
}

void LoopThreadHelper::destroyThread(LoopThread *thread) {
    if (thread && thread->hasStarted()) {
        thread->stop();
        thread->join();
        delete thread;
    }
}
