//
// Created by Gibbs on 2020/7/16.
//

#ifndef GPLAYER_MEDIASOURCEJNI_H
#define GPLAYER_MEDIASOURCEJNI_H

using namespace std;

#include <jni.h>
#include <media/Media.h>
#include <string>

class MediaSourceJni {
public:
    MediaSourceJni(jobject obj);

    ~MediaSourceJni();

private:
    jobject p2pSourceJObj{};
    jmethodID getUrlMethodId{};
    jmethodID getFlagMethodId{};
    jmethodID onInitMethodId{};
    jmethodID onReceiveAudioMethodId{};
    jmethodID onReceiveVideoMethodId{};
    jmethodID onReleaseMethodId{};
    jmethodID onErrorMethodId{};
    jmethodID onAudioPacketSizeChangedMethodId{};
    jmethodID onVideoPacketSizeChangedMethodId{};

public:

    void createJni(jobject obj);

    void destroyJni();

    int sendAudio2Java(MediaData *outFrame);

    int sendVideo2Java(MediaData *outFrame);

    void sendAudioPacketSize2Java(int size);

    void sendVideoPacketSize2Java(int size);

    void callJavaInitMethod(MediaInfo *header, int channelId);

    void callJavaReleaseMethod();

    void callJavaErrorMethod(int errorCode, const string& errorMessage);

    int getFlag();

    string getUrl();

};


#endif //GPLAYER_MEDIASOURCEJNI_H
