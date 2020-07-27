#ifndef GPLAYER_COMMONTHREAD_H
#define GPLAYER_COMMONTHREAD_H

#include "XThread.h"
#include <functional>

using namespace std;

class CommonThread : public XThread {
public:
    CommonThread(int fps);

	virtual ~CommonThread();

    void setStartFunc(std::function<void(void)> func);

    void setUpdateFunc(std::function<void(int64_t)> func);

    void setEndFunc(std::function<void(void)> func);

    bool hasStarted();

protected:
    void handleRunning();

private:
    int fps;    //帧率
    int64_t currentTime;
    int64_t frameInterval;
    std::function<void(void)> startFunc;
    std::function<void(int64_t)> updateFunc;
    std::function<void(void)> endFunc;
    bool isStarted = false;
};

#endif //GPLAYER_COMMONTHREAD_H
