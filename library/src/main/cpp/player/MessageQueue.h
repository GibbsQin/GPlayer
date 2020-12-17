//
// Created by qinshenghua on 2020/12/17.
//

#ifndef GPLAYER_MESSAGEQUEUE_H
#define GPLAYER_MESSAGEQUEUE_H

#include <deque>
#include <mutex>

#define MSG_FROM_ERROR     0
#define MSG_FROM_STATE     1
#define MSG_FROM_TIME      2
#define MSG_FROM_SIZE      3
#define MSG_FROM_DEMUXING  4
#define MSG_FROM_DECODE    5

#define MSG_DEMUXING_INIT     0
#define MSG_DEMUXING_DESTROY  1
#define MSG_DEMUXING_ERROR    2

#define MSG_COMMON_AUDIO_PACKET_SIZE 1
#define MSG_COMMON_VIDEO_PACKET_SIZE 2
#define MSG_COMMON_AUDIO_FRAME_SIZE  3
#define MSG_COMMON_VIDEO_FRAME_SIZE  4

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
    void pushMessage(int from, int type, long extra);

    int dequeMessage(Message *message);

private:
    std::deque<Message *> msgQueue;
    std::mutex messageLock;
};


#endif //GPLAYER_MESSAGEQUEUE_H
