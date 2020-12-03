package com.gibbs.gplayer.source;

import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.media.MediaInfo;

public interface MediaSource {
    MediaInfo getMediaInfo();

    MediaData readAudioSource();

    MediaData readVideoSource();

    void removeFirstAudioPackage();

    void removeFirstVideoPackage();

    void flushBuffer();

    int getAudioBufferSize();

    int getVideoBufferSize();
}
