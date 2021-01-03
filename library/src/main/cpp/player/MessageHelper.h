/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#ifndef GPLAYER_MESSAGEHELPER_H
#define GPLAYER_MESSAGEHELPER_H


#include "GPlayerJni.h"
#include "MessageSource.h"

class MessageHelper {
public:
    MessageHelper(MessageSource *messageSource, jobject obj);

    ~MessageHelper();

    void handleErrorMessage(Message *message);

    static const char *error2String(int errorCode, int errorExtra);

    void notifyJava(int msgId, int arg1, long arg2, const char* msg1, const char* msg2);

private:
    MessageSource *messageSource;
    GPlayerJni *playerJni;
};


#endif //GPLAYER_MESSAGEHELPER_H
