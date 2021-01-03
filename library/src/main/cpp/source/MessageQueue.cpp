//
// Created by qinshenghua on 2020/12/17.
//

#include <base/Log.h>
#include "MessageQueue.h"

#define NOT_NOTIFY_BUFFER_SIZE

MessageQueue::MessageQueue() = default;

MessageQueue::~MessageQueue() {
    LOGI("MessageQueue", "CoreFlow : message queue destroyed %d", msgQueue.size());
}

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
    isReset = false;
}

int MessageQueue::dequeMessage(Message **message) {
    if (isReset) {
        return -1;
    }
    if (msgQueue.size() <= 0) {
        std::unique_lock<std::mutex> lck(messageLock);
        conVar.wait(lck);
    }
    Message *msg = msgQueue.front();
    if (msg == nullptr) {
        popMessage();
        return -1;
    }
    *message = msg;
    return 0;
}

void MessageQueue::popMessage() {
    mQueueLock.lock();
    if (msgQueue.size() > 0) {
        Message *msg = msgQueue.front();
        free(msg);
        msgQueue.pop_front();
    }
    mQueueLock.unlock();
}

void MessageQueue::flush() {
    while (msgQueue.size() > 0) {
        popMessage();
    }
    LOGI("MessageQueue", "CoreFlow : flushBuffer");
}

void MessageQueue::reset() {
    std::unique_lock<std::mutex> lck(messageLock);
    conVar.notify_all();
    isReset = true;
}
