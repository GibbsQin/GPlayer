//
// Created by qinshenghua on 2020/12/18.
//

#ifndef GPLAYER_MESSAGEHELPER_H
#define GPLAYER_MESSAGEHELPER_H


#include "GPlayerJni.h"
#include "MessageQueue.h"

class MessageHelper {
public:
    MessageHelper(MessageQueue *messageQueue, jobject obj);

    ~MessageHelper();

    int processMessage(int arg1, long arg2);

    void handleMessage(Message *message);

    void notifyJava(int msgId, int arg1, long arg2, char* msg1, char* msg2);

private:
    MessageQueue *messageQueue;
    GPlayerJni *playerJni;
};


#endif //GPLAYER_MESSAGEHELPER_H
