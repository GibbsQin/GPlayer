package com.gibbs.gplayer.render;

import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.source.MediaSource;
import com.gibbs.gplayer.utils.LogUtils;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

abstract class BaseVideoRender implements VideoRender {
    private static final String TAG = "BaseVideoRender";

    static final int INCREMENT_INTERVAL_MS = 2;

    MediaSource mMediaSource;

    private OnVideoRenderChangedListener mOnVideoRenderChangedListener;
    private boolean mIsAvailable;
    AVSync mAVSync;
    long mMaxFrameIntervalMs = 0;
    long mFrameIntervalMs = 0;//帧之间的渲染间隔，平滑处理

    BaseVideoRender(MediaSource source) {
        mMediaSource = source;
    }

    @Override
    public void init(MediaInfo header) {
        LogUtils.i(TAG, "CoreFlow : init");
        int rate = header.getInteger(MediaInfo.KEY_FRAME_RATE, 20);
        mFrameIntervalMs = 1000L / rate / 2;
        mMaxFrameIntervalMs = 1000L / rate * 9 / 10;
        LogUtils.i(TAG, "init mFrameIntervalMs = " + mFrameIntervalMs + ", mMaxFrameIntervalMs = " + mMaxFrameIntervalMs);
    }

    @Override
    public void release() {
        LogUtils.i(TAG, "CoreFlow : release");
    }

    @Override
    public void setAVSync(AVSync avSync) {
        mAVSync = avSync;
    }

    @Override
    public boolean isAvailable() {
        return mIsAvailable;
    }

    @Override
    public void setOnVideoRenderChangedListener(OnVideoRenderChangedListener listener) {
        mOnVideoRenderChangedListener = listener;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        mIsAvailable = true;
        if (mOnVideoRenderChangedListener != null) {
            mOnVideoRenderChangedListener.onSurfaceCreated();
        }
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        if (mOnVideoRenderChangedListener != null) {
            mOnVideoRenderChangedListener.onSurfaceChanged(width, height);
        }
    }

    @Override
    public void onDrawFrame(GL10 gl) {

    }
}
