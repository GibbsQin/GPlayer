package com.gibbs.gplayer.source;

import com.gibbs.gplayer.listener.OnPositionChangedListener;
import com.gibbs.gplayer.media.MediaData;

public class MediaSourceImp implements MediaSource {
    private static final String TAG = "MediaSourceImpJ";

    private int mChannelId;
    private MediaData mTopAudioFrame = null;
    private MediaData mTopVideoFrame = null;
    private OnPositionChangedListener mOnPositionChangedListener;

    public MediaSourceImp(int channel) {
        this(channel, null);
    }

    public MediaSourceImp(int channel, OnPositionChangedListener listener) {
        mChannelId = channel;
        mOnPositionChangedListener = listener;
    }

    @Override
    public MediaData readAudioSource() {
        if (mTopAudioFrame == null) {
            mTopAudioFrame = nReadAudioSource(mChannelId);
            if (mOnPositionChangedListener != null && mTopAudioFrame != null) {
                mOnPositionChangedListener.onPositionChanged((int) (mTopAudioFrame.pts / 1000));
            }
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

    @Override
    public int getFrameRate() {
        return getFrameRate(mChannelId);
    }

    @Override
    public int getDuration() {
        return getDuration(mChannelId);
    }

    @Override
    public int getSampleRate() {
        return getSampleRate(mChannelId);
    }

    @Override
    public int getSampleFormat() {
        return getSampleFormat(mChannelId);
    }

    @Override
    public long getChannelLayout() {
        return getChannelLayout(mChannelId);
    }

    @Override
    public int getChannels() {
        return getChannels(mChannelId);
    }

    @Override
    public int getWidth() {
        return getWidth(mChannelId);
    }

    @Override
    public int getHeight() {
        return getHeight(mChannelId);
    }

    @Override
    public int getRotate() {
        return getRotate(mChannelId);
    }

    private native MediaData nReadAudioSource(int channelId);

    private native MediaData nReadVideoSource(int channelId);

    private native void nRemoveFirstAudioPackage(int channelId);

    private native void nRemoveFirstVideoPackage(int channelId);

    private native void nFlushBuffer(int channelId);

    private native int nGetAudioBufferSize(int channelId);

    private native int nGetVideoBufferSize(int channelId);

    private native int getFrameRate(int channelId);

    private native int getDuration(int channelId);

    private native int getSampleRate(int channelId);

    private native int getSampleFormat(int channelId);

    private native long getChannelLayout(int channelId);

    private native int getChannels(int channelId);

    private native int getWidth(int channelId);

    private native int getHeight(int channelId);

    private native int getRotate(int channelId);
}
