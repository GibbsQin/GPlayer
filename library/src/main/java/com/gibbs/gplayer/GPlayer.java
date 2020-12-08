package com.gibbs.gplayer;

import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;
import android.view.SurfaceView;

import com.gibbs.gplayer.listener.OnBufferChangedListener;
import com.gibbs.gplayer.listener.OnErrorListener;
import com.gibbs.gplayer.listener.OnPositionChangedListener;
import com.gibbs.gplayer.listener.OnPreparedListener;
import com.gibbs.gplayer.listener.OnStateChangedListener;
import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.render.AudioRender;
import com.gibbs.gplayer.render.PcmAudioRender;
import com.gibbs.gplayer.render.VideoRender;
import com.gibbs.gplayer.render.YUVGLRenderer;
import com.gibbs.gplayer.source.MediaSource;
import com.gibbs.gplayer.source.MediaSourceImp;
import com.gibbs.gplayer.utils.LogUtils;

public class GPlayer implements IGPlayer, OnPositionChangedListener {
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
    private int mCurrentPosition;

    private AudioRender mAudioRender = null;
    private VideoRender mVideoRender = null;

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
        mMediaSource = new MediaSourceImp(mChannelId, this);
        nInit(mChannelId, mediaCodec ? 2 : 0, this);
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        release();
    }

    @Override
    public void setSurface(SurfaceView view) {
        mAudioRender = new PcmAudioRender(mMediaSource);
        mVideoRender = YUVGLRenderer.createVideoRender(view, mAudioRender, mMediaSource);
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

        nStart(mChannelId);
        setPlayState(GPlayer.State.PLAYING);

        mIsProcessingSource = true;
        mAudioPlayThread = new AudioPlayThread();
        mAudioPlayThread.setName("GPlayer_AudioPlayThread");
        mAudioPlayThread.start();
        mVideoPlayThread = new VideoPlayThread();
        mVideoPlayThread.setName("GPlayer_VideoPlayThread");
        mVideoPlayThread.start();
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

    @Override
    public void release() {
        LogUtils.i(TAG, "CoreFlow : release channelId " + mChannelId);
        nRelease(mChannelId);
    }

    /**
     * is this player playing
     *
     * @return true playing, false not playing
     */
    @Override
    public boolean isPlaying() {
        return mPlayState == State.PLAYING;
    }

    @Override
    public void seekTo(int secondMs) {
        nSeekTo(mChannelId, secondMs);
    }

    @Override
    public int getCurrentPosition() {
        return mCurrentPosition;
    }

    @Override
    public int getDuration() {
        return mMediaSource.getDuration();
    }

    @Override
    public int getVideoWidth() {
        return mMediaSource.getWidth();
    }

    @Override
    public int getVideoHeight() {
        return mMediaSource.getHeight();
    }

    @Override
    public int getVideoRotate() {
        return mMediaSource.getRotate();
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

    @Override
    public void onPositionChanged(int timeUs) {
        onMessageCallback(MSG_TYPE_TIME, timeUs, 0, null, null, null);
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
        if (mOnPreparedListener != null) {
            mOnPreparedListener.onPrepared();
        }
    }

    private void onStopped() {
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
        mMediaSource.flushBuffer();
    }

    private void onReleased() {
        setPlayState(State.IDLE);
    }

    private class AudioPlayThread extends Thread {
        @Override
        public void run() {
            super.run();
            LogUtils.i(TAG, "AudioPlayThread start " + getId());
            while (mAudioRender == null) {
                if (!mIsProcessingSource) {
                    return;
                }
                while (mMediaSource.getAudioBufferSize() > 0) {
                    mMediaSource.removeFirstAudioPackage();
                }
                try {
                    sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            LogUtils.i(TAG, "CoreFlow : start audio render");
            mAudioRender.init(mMediaSource);
            while (mIsProcessingSource) {
                mAudioRender.render();
            }
            mAudioRender.release();
            LogUtils.i(TAG, "AudioPlayThread stop " + getId());
        }
    }

    private class VideoPlayThread extends Thread {
        @Override
        public void run() {
            super.run();
            LogUtils.i(TAG, "VideoPlayThread start " + getId());
            while (mVideoRender == null) {
                if (!mIsProcessingSource) {
                    return;
                }
                while (mMediaSource.getVideoBufferSize() > 0) {
                    MediaData mediaData = mMediaSource.readVideoSource();
                    if (mediaData.pts < mCurrentPosition) {
                        mMediaSource.removeFirstVideoPackage();
                    }
                }
                try {
                    sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            LogUtils.i(TAG, "CoreFlow : start video render");
            mVideoRender.init(mMediaSource);
            while (mIsProcessingSource) {
                mVideoRender.render();
            }
            mVideoRender.release();
            LogUtils.i(TAG, "VideoPlayThread stop " + getId());
        }
    }

    //call by jni
    public void onMessageCallback(final int what, final int arg1, final int arg2, final String msg1,
                                  final String msg2, final Object object) {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                switch (what) {
                    case MSG_TYPE_ERROR:
                        handleErrorMsg(arg1, msg1);
                        break;
                    case MSG_TYPE_STATE:
                        handleStateMsg(arg1);
                        break;
                    case MSG_TYPE_TIME:
                        handleTimeMsg(arg1);
                        break;
                    case MSG_TYPE_SIZE:
                        handleSizeMsg(arg1, arg2);
                        break;
                }
            }
        });
    }

    private void handleErrorMsg(int code, String errorMsg) {
        if (mOnErrorListener != null) {
            mOnErrorListener.onError(code, errorMsg);
        }
    }

    private void handleStateMsg(int state) {
        State playState = State.values()[state];
        setPlayState(playState);
    }

    private void handleTimeMsg(int time) {
        mCurrentPosition = time;
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
