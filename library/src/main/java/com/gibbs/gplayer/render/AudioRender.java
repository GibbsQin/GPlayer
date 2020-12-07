package com.gibbs.gplayer.render;

import androidx.annotation.NonNull;

import com.gibbs.gplayer.source.MediaSource;

import java.nio.ByteBuffer;

public interface AudioRender {
    void init(MediaSource mediaSource);

    void render();

    void release();

    int write(@NonNull ByteBuffer outputBuffer, int size, long presentationTimeUs, int writeMode);

    long getAudioTimeUs();
}
