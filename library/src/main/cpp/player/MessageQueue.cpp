//
// Created by qinshenghua on 2020/12/17.
//

#include "MessageQueue.h"

void MessageQueue::pushMessage(int from, int type, long extra) {
    Message *message = static_cast<Message *>(malloc(sizeof(Message)));
    message->from = from;
    message->type = type;
    message->extra = extra;
    msgQueue.push_back(message);
    if (messageLock.try_lock()) {
        messageLock.unlock();
    }
}

int MessageQueue::dequeMessage(Message *message) {
    if (msgQueue.size() <= 0) {
        messageLock.lock();
    }
    Message *msg = msgQueue.front();
    message->from = msg->from;
    message->type = msg->type;
    message->extra = msg->extra;
    free(msg);
    msgQueue.pop_front();
    return 0;
}
