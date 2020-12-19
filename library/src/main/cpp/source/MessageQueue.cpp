//
// Created by qinshenghua on 2020/12/17.
//

#include <base/Log.h>
#include "MessageQueue.h"

#define NOT_NOTIFY_BUFFER_SIZE

void MessageQueue::pushMessage(int from, int type, long extra) {
#ifdef NOT_NOTIFY_BUFFER_SIZE
    if (from == MSG_DOMAIN_BUFFER) {
        return;
    }
#endif
    auto message = static_cast<Message *>(malloc(sizeof(Message)));
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
    memcpy(message, msg, sizeof(Message));
    free(msg);
    msgQueue.pop_front();
    return 0;
}

void MessageQueue::flush() {
    if (msgQueue.size() > 0) {
        Message *msg = msgQueue.front();
        free(msg);
        msgQueue.pop_front();
    }
    LOGI("MessageQueue", "CoreFlow : flushBuffer");
}
