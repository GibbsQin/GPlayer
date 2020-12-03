package com.gibbs.gplayer.source;

import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.media.MediaInfo;

public class MediaSourceImp implements MediaSource {
    private static final String TAG = "MediaSourceImpJ";

    private int mChannelId;

    public MediaSourceImp(int channel) {
        mChannelId = channel;
    }

    @Override
    public MediaInfo getMediaInfo() {
        return nGetAVHeader(mChannelId);
    }

    @Override
    public MediaData readAudioSource() {
        return nReadAudioSource(mChannelId);
    }

    @Override
    public MediaData readVideoSource() {
        return nReadVideoSource(mChannelId);
    }

    @Override
    public void removeFirstAudioPackage() {
        nRemoveFirstAudioPackage(mChannelId);
    }

    @Override
    public void removeFirstVideoPackage() {
        nRemoveFirstVideoPackage(mChannelId);
    }

    @Override
    public void flushBuffer() {
        nFlushBuffer(mChannelId);
    }

    @Override
    public int getAudioBufferSize() {
        return nGetAudioBufferSize(mChannelId);
    }

    @Override
    public int getVideoBufferSize() {
        return nGetVideoBufferSize(mChannelId);
    }

    private native MediaInfo nGetAVHeader(int channelId);

    private native MediaData nReadAudioSource(int channelId);

    private native MediaData nReadVideoSource(int channelId);

    private native void nRemoveFirstAudioPackage(int channelId);

    private native void nRemoveFirstVideoPackage(int channelId);

    private native void nFlushBuffer(int channelId);

    private native int nGetAudioBufferSize(int channelId);

    private native int nGetVideoBufferSize(int channelId);
}
