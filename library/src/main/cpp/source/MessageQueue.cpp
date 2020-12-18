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
    std::unique_lock<std::mutex> lck(messageLock);
    conVar.notify_all();
}

int MessageQueue::dequeMessage(Message *message) {
    while (msgQueue.size() <= 0) {
        std::unique_lock<std::mutex> lck(messageLock);
        conVar.wait(lck);
    }
    Message *msg = msgQueue.front();
    if (msg == nullptr) {
        msgQueue.pop_front();
        return -1;
    }
    message->from = msg->from;
    message->type = msg->type;
    message->extra = msg->extra;
    free(msg);
    msgQueue.pop_front();
    return 0;
}

void MessageQueue::flush() {
    if (msgQueue.size() > 0) {
        msgQueue.clear();
    }
}
