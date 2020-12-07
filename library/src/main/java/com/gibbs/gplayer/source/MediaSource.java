package com.gibbs.gplayer.source;

import com.gibbs.gplayer.media.MediaData;

public interface MediaSource {
    MediaData readAudioSource();

    MediaData readVideoSource();

    void removeFirstAudioPackage();

    void removeFirstVideoPackage();

    void flushBuffer();

    int getAudioBufferSize();

    int getVideoBufferSize();

    int getFrameRate();

    long getDuration();

    int getSampleRate();

    int getSampleFormat();

    long getChannelLayout();

    int getChannels();

    int getWidth();

    int getHeight();

    int getRotate();
}
