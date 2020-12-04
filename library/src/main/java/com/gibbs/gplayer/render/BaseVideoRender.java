package com.gibbs.gplayer.render;

import android.content.Context;

import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.source.MediaSource;
import com.gibbs.gplayer.utils.LogUtils;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

abstract class BaseVideoRender implements VideoRender {
    private static final String TAG = "BaseVideoRender";

    static final int INCREMENT_INTERVAL_MS = 2;

    MediaSource mMediaSource;

    private boolean mIsAvailable;
    AVSync mAVSync;
    long mMaxFrameIntervalMs = 0;
    long mFrameIntervalMs = 0;//帧之间的渲染间隔，平滑处理

    BaseVideoRender(Context context, AudioRender audioRender, MediaSource source) {
        mMediaSource = source;
        mAVSync = new AVSync(context, audioRender);
    }

    @Override
    public void init(MediaInfo header) {
        int rate = header.getInteger(MediaInfo.KEY_FRAME_RATE, 20);
        mFrameIntervalMs = 1000L / rate / 2;
        mMaxFrameIntervalMs = 1000L / rate * 9 / 10;
        mAVSync.enable();
        LogUtils.i(TAG, "init mFrameIntervalMs = " + mFrameIntervalMs + ", mMaxFrameIntervalMs = " + mMaxFrameIntervalMs);
    }

    @Override
    public void release() {
        LogUtils.i(TAG, "release");
        mAVSync.disable();
    }

    @Override
    public boolean isAvailable() {
        return mIsAvailable;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        mIsAvailable = true;
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
    }

    @Override
    public void onDrawFrame(GL10 gl) {

    }
}
