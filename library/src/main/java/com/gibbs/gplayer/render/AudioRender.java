package com.gibbs.gplayer.render;

import com.gibbs.gplayer.media.MediaInfo;

import java.nio.ByteBuffer;

import androidx.annotation.NonNull;

public interface AudioRender {
    void init(MediaInfo mediaInfo);

    void render();

    void release();

    int write(@NonNull ByteBuffer outputBuffer, int size, long presentationTimeUs, int writeMode);

    long getAudioTimeUs();
}
