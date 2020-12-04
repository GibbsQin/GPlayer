package com.gibbs.gplayer;

import android.opengl.GLSurfaceView;
import android.os.Handler;
import android.text.TextUtils;
import android.view.SurfaceView;

import com.gibbs.gplayer.listener.OnErrorListener;
import com.gibbs.gplayer.listener.OnBufferChangedListener;
import com.gibbs.gplayer.listener.OnPreparedListener;
import com.gibbs.gplayer.listener.OnStateChangedListener;
import com.gibbs.gplayer.listener.OnPositionChangedListener;
import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.render.AudioRender;
import com.gibbs.gplayer.render.PcmAudioRender;
import com.gibbs.gplayer.render.VideoRender;
import com.gibbs.gplayer.render.YUVGLRenderer;
import com.gibbs.gplayer.source.MediaSource;
import com.gibbs.gplayer.source.MediaSourceImp;
import com.gibbs.gplayer.utils.LogUtils;

public class GPlayer implements IGPlayer {
    private static final String TAG = "GPlayerJ";

    private static final int MSG_TYPE_ERROR = 0;
    private static final int MSG_TYPE_STATE = 1;
    private static final int MSG_TYPE_TIME = 2;
    private static final int MSG_TYPE_SIZE = 3;

    static {
        System.loadLibrary("gplayer");
    }

    public enum State {
        IDLE, PREPARING, PREPARED, PAUSED, PLAYING, STOPPING, STOPPED, RELEASED
    }

    private int mChannelId = hashCode();
    private String mUrl;
    private MediaSource mMediaSource;
    private AudioPlayThread mAudioPlayThread;
    private VideoPlayThread mVideoPlayThread;
    private boolean mIsProcessingSource;
    private State mPlayState = State.IDLE;

    private SurfaceView mSurfaceView;
    private AudioRender mAudioRender;
    private VideoRender mVideoRender;

    private OnPreparedListener mOnPreparedListener;
    private OnErrorListener mOnErrorListener;
    private OnStateChangedListener mOnStateChangedListener;
    private OnPositionChangedListener mOnPositionChangedListener;
    private OnBufferChangedListener mOnBufferChangedListener;

    public GPlayer() {
        this(false);
    }

    public GPlayer(boolean mediaCodec) {
        LogUtils.i(TAG, "CoreFlow : new GPlayer " + mediaCodec);
        mMediaSource = new MediaSourceImp(mChannelId);
        nInit(mChannelId, mediaCodec ? 2 : 0, this);
    }

    @Override
    public void setSurface(SurfaceView view) {
        mSurfaceView = view;
        mAudioRender = new PcmAudioRender(mMediaSource);
        if (view instanceof GLSurfaceView) {
            GLSurfaceView glSurfaceView = (GLSurfaceView) view;
            mVideoRender = new YUVGLRenderer(glSurfaceView, mAudioRender, mMediaSource);
            glSurfaceView.setEGLContextClientVersion(2);
            glSurfaceView.setRenderer(mVideoRender);
            glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        }
    }

    @Override
    public void setDataSource(String url) {
        mUrl = url;
    }

    /**
     * start to play
     */
    @Override
    public void prepare() {
        if (mPlayState != State.IDLE) {
            LogUtils.e(TAG, "CoreFlow : not idle");
            return;
        }
        if (TextUtils.isEmpty(mUrl)) {
            LogUtils.e(TAG, "CoreFlow : invalid data source");
            return;
        }
        LogUtils.i(TAG, "CoreFlow : prepare " + mUrl);
        nPrepare(mChannelId, mUrl);
    }

    @Override
    public void start() {
        if (mPlayState != State.PREPARED && mPlayState != State.PAUSED) {
            LogUtils.e(TAG, "CoreFlow : not prepared or paused");
            return;
        }
        if (!mSurfaceView.getHolder().getSurface().isValid()) {
            LogUtils.e(TAG, "CoreFlow : invalid surface");
            // TODO
            try {
                Thread.sleep(200);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            LogUtils.e(TAG, "CoreFlow : try again after 200 ms");
            start();
            return;
        }

        mIsProcessingSource = true;
        setPlayState(GPlayer.State.PLAYING);

        mAudioPlayThread = new AudioPlayThread();
        mAudioPlayThread.setName("GPlayer_AudioPlayThread");
        mAudioPlayThread.start();
        mVideoPlayThread = new VideoPlayThread();
        mAudioPlayThread.setName("GPlayer_VideoPlayThread");
        mVideoPlayThread.start();

        nStart(mChannelId);
    }

    @Override
    public void stop() {
        if (mPlayState != State.PREPARING && mPlayState != State.PREPARED &&
                mPlayState != State.PAUSED && mPlayState != State.PLAYING) {
            LogUtils.e(TAG, "CoreFlow : not playing");
            return;
        }
        LogUtils.i(TAG, "CoreFlow : stop");
        setPlayState(State.STOPPING);
        nStop(mChannelId, true);
    }

    @Override
    public void pause() {
        nPause(mChannelId);
    }

    /**
     * is this player playing
     *
     * @return true playing, false not playing
     */
    public boolean isPlaying() {
        return mPlayState == State.PLAYING;
    }

    @Override
    public void seekTo(int secondMs) {
        nSeekTo(mChannelId, secondMs);
    }

    @Override
    public int getCurrentPosition() {
        return 0;
    }

    @Override
    public int getDuration() {
        return 0;
    }

    @Override
    public void setOnPreparedListener(OnPreparedListener listener) {
        mOnPreparedListener = listener;
    }

    @Override
    public void setOnErrorListener(OnErrorListener listener) {
        mOnErrorListener = listener;
    }

    @Override
    public void setOnStateChangedListener(OnStateChangedListener listener) {
        mOnStateChangedListener = listener;
    }

    @Override
    public void setOnPositionChangedListener(OnPositionChangedListener listener) {
        mOnPositionChangedListener = listener;
    }

    @Override
    public void setOnBufferChangedListener(OnBufferChangedListener listener) {
        mOnBufferChangedListener = listener;
    }

    public MediaInfo getMediaInfo() {
        return mMediaSource.getMediaInfo();
    }

    public String getUrl() {
        return mUrl;
    }

    private void setPlayState(State state) {
        synchronized (this) {
            if (state == mPlayState) {
                return;
            }
            mPlayState = state;
        }
        LogUtils.e(TAG, "CoreFlow : setPlayState state = " + state);
        switch (state) {
            case PREPARED:
                onPrepared();
                break;
            case STOPPED:
                onStopped();
                break;
            case RELEASED:
                onReleased();
                break;
        }
        if (mOnStateChangedListener != null) {
            mOnStateChangedListener.onStateChanged(mPlayState);
        }
    }

    private void onPrepared() {
        LogUtils.i(TAG, "CoreFlow : ----------onPrepared----------");
        if (mOnPreparedListener != null) {
            mOnPreparedListener.onPrepared();
        }
    }

    private void onStopped() {
        LogUtils.e(TAG, "CoreFlow : ----------onStopped----------");
        mIsProcessingSource = false;
        if (mAudioPlayThread != null && mAudioPlayThread.isAlive()) {
            try {
                mAudioPlayThread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        if (mVideoPlayThread != null && mVideoPlayThread.isAlive()) {
            try {
                mVideoPlayThread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        // TODO
        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                mMediaSource.flushBuffer();
                release();
            }
        }, 200);
    }

    private void onReleased() {
        LogUtils.e(TAG, "CoreFlow : ----------onReleased----------");
        setPlayState(State.IDLE);
    }

    private class AudioPlayThread extends Thread {
        @Override
        public void run() {
            super.run();
            LogUtils.i(TAG, "AudioPlayThread init " + getId());
            mAudioRender.init(mMediaSource.getMediaInfo());
            while (mIsProcessingSource) {
                mAudioRender.render();
            }
            mAudioRender.release();
            LogUtils.i(TAG, "AudioPlayThread end " + getId());
        }
    }

    private class VideoPlayThread extends Thread {
        @Override
        public void run() {
            super.run();
            LogUtils.i(TAG, "VideoPlayThread init " + getId());
            mVideoRender.init(mMediaSource.getMediaInfo());
            while (mIsProcessingSource) {
                mVideoRender.render();
            }
            mVideoRender.release();
            LogUtils.i(TAG, "VideoPlayThread end " + getId());
        }
    }

    private void release() {
        LogUtils.i(TAG, "CoreFlow : release channelId " + mChannelId);
        nRelease(mChannelId);
    }

    //call by jni
    public void onMessageCallback(int what, int arg1, int arg2, String msg1, String msg2, Object object) {
        switch (what) {
            case MSG_TYPE_ERROR:
                handleErrorMsg(arg1, msg1);
                break;
            case MSG_TYPE_STATE:
                handleStateMsg(arg1, object);
                break;
            case MSG_TYPE_TIME:
                handleTimeMsg(arg1);
                break;
            case MSG_TYPE_SIZE:
                handleSizeMsg(arg1, arg2);
                break;
        }
    }

    private void handleErrorMsg(int code, String errorMsg) {
        if (mOnErrorListener != null) {
            mOnErrorListener.onError(code, errorMsg);
        }
    }

    private void handleStateMsg(int state, Object object) {
        if (object instanceof MediaInfo) {
            mMediaSource.setMediaInfo((MediaInfo) object);
        }
        State playState = State.values()[state];
        setPlayState(playState);
    }

    private void handleTimeMsg(int time) {
        if (mOnPositionChangedListener != null) {
            mOnPositionChangedListener.onPositionChanged(time);
        }
    }

    private void handleSizeMsg(int type, int size) {
        if (mOnBufferChangedListener != null) {
            if (type == 1) {
                mOnBufferChangedListener.onAudioPacketSizeChanged(size);
            } else if (type == 2) {
                mOnBufferChangedListener.onVideoPacketSizeChanged(size);
            } else if (type == 3) {
                mOnBufferChangedListener.onAudioFrameSizeChanged(size);
            } else if (type == 4) {
                mOnBufferChangedListener.onVideoFrameSizeChanged(size);
            }
        }
    }

    // 创建GPlayer.c实例
    private native void nInit(int channelId, int flag, GPlayer player);

    // 开始解封装
    private native void nPrepare(int channelId, String url);

    // 开始解码
    private native void nStart(int channelId);

    // 暂停解码和解封装
    private native void nPause(int channelId);

    // seek
    private native void nSeekTo(int channelId, int secondMs);

    // 停止解封装和解码
    private native void nStop(int channelId, boolean force);

    // 释放GPlayer.c实例
    private native void nRelease(int channelId);
}
