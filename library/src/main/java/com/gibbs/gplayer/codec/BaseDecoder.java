package com.gibbs.gplayer.codec;

import android.media.MediaCodec;

import com.gibbs.gplayer.media.MediaInfo;

import java.nio.ByteBuffer;

interface BaseDecoder {
    void init(MediaInfo header);

    boolean feedInputBuffer();

    boolean drainOutputBuffer();

    void renderBuffer();

    void release();

    class MediaCodecOutputBuffer {
        ByteBuffer outputBuffer;
        int bufferId;
        MediaCodec.BufferInfo bufferInfo;

        MediaCodecOutputBuffer(ByteBuffer outputBuffer, int bufferId, MediaCodec.BufferInfo bufferInfo) {
            this.outputBuffer = outputBuffer;
            this.bufferId = bufferId;
            this.bufferInfo = bufferInfo;
        }
    }
}
