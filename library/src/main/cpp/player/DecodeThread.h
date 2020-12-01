#ifndef GPLAYER_DECODETHREAD_H
#define GPLAYER_DECODETHREAD_H

#include "XThread.h"
#include <functional>

#define MAX_OUTPUT_FRAME_SIZE 5
#define SLEEP_TIME_GAP 2

using namespace std;

class DecodeThread : public XThread {
public:
    DecodeThread();

	virtual ~DecodeThread();

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

#endif //GPLAYER_DECODETHREAD_H
