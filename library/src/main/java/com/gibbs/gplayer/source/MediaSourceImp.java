package com.gibbs.gplayer.source;

import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.media.MediaInfo;

public class MediaSourceImp implements MediaSource {
    private static final String TAG = "MediaSourceImpJ";

    private int mChannelId;
    private MediaData mTopAudioFrame = null;
    private MediaData mTopVideoFrame = null;

    public MediaSourceImp(int channel) {
        mChannelId = channel;
    }

    @Override
    public MediaInfo getMediaInfo() {
        return nGetMediaInfo(mChannelId);
    }

    @Override
    public MediaData readAudioSource() {
        if (mTopAudioFrame == null) {
            mTopAudioFrame = nReadAudioSource(mChannelId);
        }
        return mTopAudioFrame;
    }

    @Override
    public MediaData readVideoSource() {
        if (mTopVideoFrame == null) {
            mTopVideoFrame = nReadVideoSource(mChannelId);
        }
        return mTopVideoFrame;
    }

    @Override
    public void removeFirstAudioPackage() {
        mTopAudioFrame = null;
        nRemoveFirstAudioPackage(mChannelId);
    }

    @Override
    public void removeFirstVideoPackage() {
        mTopVideoFrame = null;
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

    private native MediaInfo nGetMediaInfo(int channelId);

    private native MediaData nReadAudioSource(int channelId);

    private native MediaData nReadVideoSource(int channelId);

    private native void nRemoveFirstAudioPackage(int channelId);

    private native void nRemoveFirstVideoPackage(int channelId);

    private native void nFlushBuffer(int channelId);

    private native int nGetAudioBufferSize(int channelId);

    private native int nGetVideoBufferSize(int channelId);
}
