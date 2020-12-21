//
// Created by qinshenghua on 2020/12/17.
//

#ifndef GPLAYER_MESSAGEQUEUE_H
#define GPLAYER_MESSAGEQUEUE_H

#include <deque>
#include <mutex>
#include <condition_variable>

#define MSG_DOMAIN_ERROR     0
#define MSG_DOMAIN_STATE     1
#define MSG_DOMAIN_TIME      2
#define MSG_DOMAIN_BUFFER    3
#define MSG_DOMAIN_DEMUXING  4
#define MSG_DOMAIN_DECODING  5

#define MSG_ERROR_DEMUXING    0
#define MSG_ERROR_DECODING    1

#define MSG_DEMUXING_INIT     0
#define MSG_DEMUXING_DESTROY  1

#define MSG_BUFFER_AUDIO_PACKET 1
#define MSG_BUFFER_VIDEO_PACKET 2
#define MSG_BUFFER_AUDIO_FRAME  3
#define MSG_BUFFER_VIDEO_FRAME  4
#define MSG_BUFFER_EMPTY        5
#define MSG_BUFFER_ENOUGH       6
#define MSG_BUFFER_OVERLOAD     7

#define STATE_IDLE      0
#define STATE_PREPARING 1
#define STATE_PREPARED  2
#define STATE_PAUSED    3
#define STATE_PLAYING   4
#define STATE_STOPPING  5
#define STATE_STOPPED   6
#define STATE_RELEASED  7


typedef struct Message {
    int from;
    int type;
    long extra;
} Message;

class MessageQueue {
public:
    MessageQueue();

    ~MessageQueue();

    void pushMessage(int from, int type, long extra);

    int dequeMessage(Message **message);

    void popMessage();

    void flush();

    void reset();

private:
    std::deque<Message *> msgQueue;
    std::mutex messageLock;
    std::condition_variable conVar;
    bool isReset = false;
    std::mutex mQueueLock;
};


#endif //GPLAYER_MESSAGEQUEUE_H
