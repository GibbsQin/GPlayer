//
// Created by qinshenghua on 2020/12/18.
//

#include <base/Log.h>
#include "MessageHelper.h"
extern "C" {
#include <codec/ffmpeg/libavutil/error.h>
}

MessageHelper::MessageHelper(MessageQueue *messageQueue, jobject obj) {
    this->messageQueue = messageQueue;
    playerJni = new GPlayerJni(obj);
}

MessageHelper::~MessageHelper() {
    notifyJava(MSG_DOMAIN_STATE, STATE_RELEASED, 0, nullptr, nullptr);
    delete playerJni;
}

int MessageHelper::processMessage(int arg1, long arg2) {
    Message *message;
    if (messageQueue->dequeMessage(&message) < 0) {
        return 0;
    }
    LOGI("MessageHelper", "processMessage %d, %d, %lld", message->from, message->type, message->extra);
    handleMessage(message);
    messageQueue->popMessage();
    return 0;
}

void MessageHelper::handleMessage(Message *message) {
    if (message->from == MSG_DOMAIN_STATE) {
        playerJni->onMessageCallback(MSG_DOMAIN_STATE, message->type, message->extra, nullptr, nullptr);
    } else if (message->from == MSG_DOMAIN_BUFFER) {
        playerJni->onMessageCallback(MSG_DOMAIN_BUFFER, message->type, message->extra, nullptr, nullptr);
    } else if (message->from == MSG_DOMAIN_ERROR) {
        if (message->type == MSG_ERROR_DEMUXING) {
            playerJni->onMessageCallback(MSG_DOMAIN_ERROR, message->type, message->extra,
                                         av_err2str((int)message->extra), nullptr);
        }
    } else if (message->from == MSG_DOMAIN_DEMUXING) {
        if (message->type == MSG_DEMUXING_INIT) {
            playerJni->onMessageCallback(MSG_DOMAIN_STATE, STATE_PREPARED, 0, nullptr, nullptr);
        }
    }
}

void MessageHelper::notifyJava(int msgId, int arg1, long arg2, char *msg1, char *msg2) {
    playerJni->onMessageCallback(msgId, arg1, arg2, msg1, msg2);
}
