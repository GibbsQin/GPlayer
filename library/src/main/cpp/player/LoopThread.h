#ifndef GPLAYER_LOOPTHREAD_H
#define GPLAYER_LOOPTHREAD_H

#include "XThread.h"
#include <functional>

#define MAX_OUTPUT_FRAME_SIZE 5
#define SLEEP_TIME_GAP 10
#define MAX_SLEEP_TIME 50

using namespace std;

class LoopThread : public XThread {
public:
    LoopThread();

	virtual ~LoopThread();

    void setStartFunc(std::function<void(void)> func);

    void setUpdateFunc(std::function<int(void)> func);

    void setEndFunc(std::function<void(void)> func);

    bool hasStarted();

protected:
    void handleRunning();

private:
    int64_t sleepTimeMs;
	std::function<void(void)> startFunc;
	std::function<int(void)> updateFunc;
    std::function<void(void)> endFunc;
    bool isStarted = false;
};

#endif //GPLAYER_LOOPTHREAD_H
