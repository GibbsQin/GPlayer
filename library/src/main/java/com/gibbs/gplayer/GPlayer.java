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

public class GPlayer implements IGPlayer {
    private static final String TAG = "GPlayerJ";

    private static final int MSG_TYPE_ERROR = 0;
    private static final int MSG_TYPE_STATE = 1;
    private static final int MSG_TYPE_TIME = 2;
    private static final int MSG_TYPE_SIZE = 3;

    public static final int USE_MEDIA_CODEC = 2;

    static {
        System.loadLibrary("gplayer");
    }

    public enum State {
        IDLE, PREPARING, PREPARED, PAUSED, PLAYING, STOPPING, STOPPED, RELEASED
    }

    private long mNativePlayer;
    private String mUrl;
    private MediaSource mMediaSource;
    private AudioPlayThread mAudioPlayThread;
    private VideoPlayThread mVideoPlayThread;
    private boolean mIsProcessingSource;
    private State mPlayState = State.IDLE;
    private State mTargetState = State.IDLE;
    private int mCurrentPositionUs;
    private Handler mHandler = new Handler(Looper.getMainLooper());

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
        mNativePlayer = nInit(mediaCodec ? USE_MEDIA_CODEC : 0, this);
        mMediaSource = new MediaSourceImp(mNativePlayer);
    }

    @Override
    public void setSurface(SurfaceView view) {
        mVideoRender = YUVGLRenderer.createVideoRender(view, mMediaSource);
    }

    @Override
    public void setDataSource(String url) {
        mUrl = url;
    }

    /**
     * start to play
     */
    @Override
    public synchronized void prepare() {
        if (mPlayState != State.IDLE && mPlayState != State.STOPPED) {
            LogUtils.e(TAG, "CoreFlow : not idle");
            return;
        }
        if (TextUtils.isEmpty(mUrl)) {
            LogUtils.e(TAG, "CoreFlow : invalid data source");
            return;
        }
        LogUtils.i(TAG, "CoreFlow : prepare " + mUrl);
        setPlayState(State.PREPARING);
        nPrepare(mNativePlayer, mUrl);
    }

    @Override
    public synchronized void start() {
        if (mPlayState == State.PREPARED) {
            LogUtils.i(TAG, "CoreFlow : start");
            nStart(mNativePlayer);

            mIsProcessingSource = true;
            mAudioPlayThread = new AudioPlayThread();
            mAudioPlayThread.setName("GPlayer_AudioPlayThread");
            mAudioPlayThread.start();
            mVideoPlayThread = new VideoPlayThread();
            mVideoPlayThread.setName("GPlayer_VideoPlayThread");
            mVideoPlayThread.start();
        } else if (mPlayState == State.PAUSED) {
            LogUtils.i(TAG, "CoreFlow : resume");
            nResume(mNativePlayer);
            setPlayState(GPlayer.State.PLAYING);
        }
    }

    @Override
    public synchronized void stop() {
        if (mPlayState != State.PREPARING && mPlayState != State.PREPARED &&
                mPlayState != State.PAUSED && mPlayState != State.PLAYING) {
            LogUtils.e(TAG, "CoreFlow : not playing");
            return;
        }
        setPlayState(State.STOPPING);
        nStop(mNativePlayer, true);
    }

    @Override
    public synchronized void pause() {
        LogUtils.i(TAG, "CoreFlow : pause");
        nPause(mNativePlayer);
        setPlayState(State.PAUSED);
    }

    @Override
    public synchronized void release() {
        if (mPlayState == State.STOPPED) {
            LogUtils.i(TAG, "CoreFlow : release channelId " + mNativePlayer);
            nRelease(mNativePlayer);
        } else {
            LogUtils.i(TAG, "CoreFlow : target to release");
            mTargetState = State.RELEASED;
        }
    }

    @Override
    public synchronized void seekTo(int secondMs) {
        LogUtils.i(TAG, "CoreFlow : seekTo " + secondMs);
        nSeekTo(mNativePlayer, secondMs);
    }

    @Override
    public boolean isPlaying() {
        return mPlayState == State.PLAYING;
    }

    @Override
    public void setFlags(int flags) {
        nSetFlags(mNativePlayer, flags);
    }

    @Override
    public int getCurrentPosition() {
        return mCurrentPositionUs;
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
    public State getState() {
        return mPlayState;
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

    public String getUrl() {
        return mUrl;
    }

    private synchronized void setPlayState(State state) {
        if (mPlayState == state) {
            return;
        }
        LogUtils.e(TAG, "CoreFlow : setPlayState from " + mPlayState + " to " + state);
        mPlayState = state;
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
        mMediaSource.init();
        mAudioRender = new PcmAudioRender(mMediaSource);
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
        if (mTargetState == State.RELEASED) {
            release();
        }
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
            long renderTime;
            while (mIsProcessingSource) {
                renderTime = mAudioRender.render();
                if (renderTime > 0) {
                    onMessageCallback(MSG_TYPE_TIME, (int) renderTime, 0, null, null, null);
                }
            }
            mAudioRender.release();
            LogUtils.i(TAG, "CoreFlow : stop audio render");
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
                    if (mediaData.pts < mCurrentPositionUs) {
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
                long renderTime = mVideoRender.render();
                notifyRenderStarted(renderTime);
            }
            mVideoRender.release();
            LogUtils.i(TAG, "CoreFlow : stop video render");
        }
    }

    private void notifyRenderStarted(long renderTime) {
        if (renderTime > 0 && mPlayState == State.PREPARED) {
            onMessageCallback(MSG_TYPE_STATE, State.PLAYING.ordinal(), 0, null, null, null);
        }
    }

    //call by jni
    public void onMessageCallback(final int what, final int arg1, final int arg2, final String msg1,
                                  final String msg2, final Object object) {
        mHandler.post(new Runnable() {
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
        LogUtils.i(TAG, "CoreFlow : handleStateMsg " + playState);
        setPlayState(playState);
    }

    private void handleTimeMsg(int time) {
        if (mCurrentPositionUs == time) {
            return;
        }
        mCurrentPositionUs = time;
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
    private native long nInit(int flag, GPlayer player);

    // 开始解封装
    private native void nPrepare(long nativePlayer, String url);

    // 开始解码
    private native void nStart(long nativePlayer);

    // 暂停解码和解封装
    private native void nPause(long nativePlayer);

    // 恢复解码和解封装
    private native void nResume(long nativePlayer);

    // seek
    private native void nSeekTo(long nativePlayer, int secondMs);

    // 停止解封装和解码
    private native void nStop(long nativePlayer, boolean force);

    // 释放GPlayer.c实例
    private native void nRelease(long nativePlayer);

    private native void nSetFlags(long nativePlayer, int flags);
}
