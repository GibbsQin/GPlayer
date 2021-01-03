/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#include <base/Log.h>
#include <thread>
#include "MessageHelper.h"

extern "C" {
#include <codec/ffmpeg/libavutil/error.h>
}

MessageHelper::MessageHelper(MessageSource *messageSource, jobject obj) {
    this->messageSource = messageSource;
    playerJni = new GPlayerJni(obj);
}

MessageHelper::~MessageHelper() {
    notifyJava(MSG_DOMAIN_STATE, STATE_RELEASED, 0, nullptr, nullptr);
    delete playerJni;
}

void MessageHelper::handleErrorMessage(Message *message) {
    const char *errorMsg = error2String(message->type, (int)message->extra);
    playerJni->onMessageCallback(MSG_DOMAIN_ERROR, message->type, message->extra, errorMsg, nullptr);
}

const char *MessageHelper::error2String(int errorCode, int errorExtra) {
    const char *errorMsg = nullptr;
    if (errorCode == MSG_ERROR_DEMUXING || errorCode == MSG_ERROR_DECODING) {
        if (errorExtra < 0) {
            errorMsg = av_err2str(errorExtra);
        } else if (errorExtra == MSG_ERROR_DEMUXING) {
            errorMsg = "demuxing error";
        } else if (errorExtra == MSG_ERROR_DECODING) {
            errorMsg = "decoding error";
        } else if (errorExtra == MSG_ERROR_SEEK) {
            errorMsg = "seek error";
        } else {
            errorMsg = "unknown error";
        }
    }
    return errorMsg;
}

void MessageHelper::notifyJava(int msgId, int arg1, long arg2, const char *msg1, const char *msg2) {
    playerJni->onMessageCallback(msgId, arg1, arg2, msg1, msg2);
}
