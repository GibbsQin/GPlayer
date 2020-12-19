#ifndef GPLAYER_LOOPTHREAD_H
#define GPLAYER_LOOPTHREAD_H

#include "XThread.h"
#include <functional>
#include <mutex>
#include <condition_variable>

#define MAX_VALUE 5
#define SLEEP_TIME_GAP 10
#define ERROR_EXIST -100

using namespace std;

class LoopThread : public XThread {
public:
    LoopThread();

    LoopThread(int maxSize);

	virtual ~LoopThread();

    void setStartFunc(std::function<void(void)> func);

    void setUpdateFunc(std::function<int(int, long)> func);

    void setEndFunc(std::function<void(void)> func);

    void resume() override ;

    bool stop() override ;

    bool hasStarted() {
		return isStarted;
    }

    void setArgs(int arg1, long arg2) {
		this->arg1 = arg1;
		this->arg2 = arg2;
    }

protected:
    void handleRunning();

private:
    int maxValue;
    int64_t sleepTimeMs = 0;
	std::function<void(void)> startFunc;
	std::function<int(int, long)> updateFunc;
    std::function<void(void)> endFunc;
    bool isStarted = false;
	std::mutex threadLock;
	std::condition_variable conVar;

    int arg1 = -1;
    long arg2 = -1;
};

#endif //GPLAYER_LOOPTHREAD_H
