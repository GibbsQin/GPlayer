//
// Created by Gibbs on 2020/12/20.
//

#ifndef GPLAYER_AUDIOTRACKJNI_H
#define GPLAYER_AUDIOTRACKJNI_H


#include "../source/FrameSource.h"

class AudioTrackJni {
public:
    AudioTrackJni(jobject obj);

    ~AudioTrackJni();

    void start(int sampleRate, int sampleFormat, int channels, int bytesPerSample);

    int write(uint8_t *buffer, int size);

    void stop();

private:
    jclass audioTrackClass;
    jmethodID audioTrackStart;
    jmethodID audioTrackWrite;
    jmethodID audioTrackStop;

private:
    jobject audioTrackObj;
};


#endif //GPLAYER_AUDIOTRACKJNI_H
