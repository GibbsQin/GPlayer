//
// Created by qinshenghua on 2020/12/18.
//

#ifndef GPLAYER_LOOPTHREADHELPER_H
#define GPLAYER_LOOPTHREADHELPER_H


#include "LoopThread.h"
#include <functional>

class LoopThreadHelper {
public:
    static LoopThread *createLoopThread(const std::function<int(int, long)>& updateFunc);

    static LoopThread *createLoopThread(int maxValue, const std::function<int(int, long)>& updateFunc);

    static LoopThread *createLoopThread(int maxValue,
                                        const std::function<void(void)>& startFunc,
                                        const std::function<int(int, long)>& updateFunc,
                                        const std::function<void(void)>& endFunc);

    static void destroyThread(LoopThread **thread);
};


#endif //GPLAYER_LOOPTHREADHELPER_H
