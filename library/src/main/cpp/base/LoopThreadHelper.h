//
// Created by qinshenghua on 2020/12/18.
//

#ifndef GPLAYER_LOOPTHREADHELPER_H
#define GPLAYER_LOOPTHREADHELPER_H


#include "LoopThread.h"
#include <functional>

class LoopThreadHelper {
public:
    static LoopThread *createLoopThread(std::function<int(int, long)> updateFunc);

    static LoopThread *createLoopThread(int maxValue, std::function<int(int, long)> updateFunc);

    static LoopThread *createLoopThread(int maxValue,
                                        std::function<void(void)> startFunc,
                                        std::function<int(int, long)> updateFunc,
                                        std::function<void(void)> endFunc);

    static void destroyThread(LoopThread *thread);
};


#endif //GPLAYER_LOOPTHREADHELPER_H
