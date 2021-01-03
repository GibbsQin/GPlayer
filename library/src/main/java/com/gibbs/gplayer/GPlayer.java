package com.gibbs.gplayer;

import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;
import android.view.Surface;
import android.view.SurfaceHolder;

import androidx.annotation.NonNull;

import java.nio.ByteBuffer;

public class GPlayer {
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

    private final long mNativePlayer;
    private final Handler mHandler = new Handler(Looper.getMainLooper());
    private String mUrl;
    private State mPlayState = State.IDLE;
    private State mTargetState = State.IDLE;
    private int mCurrentPositionUs;
    private GAudioTrack mGAudioTrack;
    private final Object mStateLock = new Object();

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
    }

    public void setSurface(Surface surface) {
        nSetSurface(mNativePlayer, surface);
    }

    public void setSurfaceHolder(SurfaceHolder holder) {
        nSetSurface(mNativePlayer, holder.getSurface());
    }

    public void setGAudioTrack(GAudioTrack audioTrack) {
        mGAudioTrack = audioTrack;
    }

    public void setDataSource(String url) {
        mUrl = url;
    }

    /**
     * start to play
     */
    public synchronized void prepare() {
        if (mPlayState != State.IDLE && mPlayState != State.STOPPED) {
            LogUtils.e(TAG, "CoreFlow : not idle");
            return;
        }
        if (TextUtils.isEmpty(mUrl)) {
            LogUtils.e(TAG, "CoreFlow : invalid data source");
            return;
        }
        setPlayState(State.PREPARING);
        nPrepare(mNativePlayer, mUrl);
    }

    public synchronized void start() {
        if (mPlayState == State.PREPARED) {
            if (mGAudioTrack == null) {
                mGAudioTrack = new DefaultGAudioTrack();
            }
            nSetAudioTrack(mNativePlayer, mGAudioTrack);
            nStart(mNativePlayer);
        } else if (mPlayState == State.PAUSED) {
            nResume(mNativePlayer);
            setPlayState(GPlayer.State.PLAYING);
        }
    }

    public synchronized void stop() {
        if (mPlayState != State.PREPARING && mPlayState != State.PREPARED &&
                mPlayState != State.PAUSED && mPlayState != State.PLAYING) {
            LogUtils.e(TAG, "CoreFlow : not playing");
            return;
        }
        setPlayState(State.STOPPING);
        nStop(mNativePlayer, true);
    }

    public synchronized void pause() {
        LogUtils.i(TAG, "CoreFlow : pause");
        nPause(mNativePlayer);
        setPlayState(State.PAUSED);
    }

    public synchronized void release() {
        if (mPlayState == State.STOPPED) {
            LogUtils.i(TAG, "CoreFlow : release channelId " + mNativePlayer);
            nRelease(mNativePlayer);
        } else {
            LogUtils.i(TAG, "CoreFlow : target to release");
            mTargetState = State.RELEASED;
        }
    }

    public synchronized void seekTo(int secondMs) {
        LogUtils.i(TAG, "CoreFlow : seekTo " + secondMs);
        nSeekTo(mNativePlayer, secondMs);
    }

    public boolean isPlaying() {
        return mPlayState == State.PLAYING;
    }

    public void setFlags(int flags) {
        nSetFlags(mNativePlayer, flags);
    }

    public int getCurrentPosition() {
        return mCurrentPositionUs;
    }

    public State getState() {
        return mPlayState;
    }

    public void setOnPreparedListener(OnPreparedListener listener) {
        mOnPreparedListener = listener;
    }

    public interface OnPreparedListener {
        void onPrepared();
    }

    public void setOnErrorListener(OnErrorListener listener) {
        mOnErrorListener = listener;
    }

    public interface OnErrorListener {
        void onError(int errorCode, String errorMessage);
    }

    public void setOnStateChangedListener(OnStateChangedListener listener) {
        mOnStateChangedListener = listener;
    }

    public interface OnStateChangedListener {
        void onStateChanged(GPlayer.State state);
    }

    public void setOnPositionChangedListener(OnPositionChangedListener listener) {
        mOnPositionChangedListener = listener;
    }

    /**
     * playing timestamp
     */
    public interface OnPositionChangedListener {
        /**
         * video timestamp changed
         *
         * @param timeMs timestamp
         */
        void onPositionChanged(int timeMs);
    }

    public void setOnBufferChangedListener(OnBufferChangedListener listener) {
        mOnBufferChangedListener = listener;
    }

    /**
     * media buffer size listener
     */
    public interface OnBufferChangedListener {
        /**
         * audio decoded frame size changed
         *
         * @param size frame size
         */
        void onAudioFrameSizeChanged(int size);

        /**
         * video decoded frame size changed
         *
         * @param size frame size
         */
        void onVideoFrameSizeChanged(int size);

        /**
         * audio compressed frame size changed
         *
         * @param size frame size
         */
        void onAudioPacketSizeChanged(int size);

        /**
         * video compressed frame size changed
         *
         * @param size frame size
         */
        void onVideoPacketSizeChanged(int size);
    }

    public String getUrl() {
        return mUrl;
    }

    private void setPlayState(State state) {
        synchronized (mStateLock) {
            if (mPlayState == state) {
                return;
            }
            LogUtils.e(TAG, "CoreFlow : setPlayState from " + mPlayState + " to " + state);
            mPlayState = state;
        }
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
        if (mTargetState == State.RELEASED) {
            release();
        }
    }

    private void onReleased() {
        setPlayState(State.IDLE);
    }

    //call by jni
    public void onMessageCallback(final int what, final int arg1, final long arg2, final String msg1,
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
                        handleSizeMsg(arg1, (int) arg2);
                        break;
                }
            }
        });
    }

    private void handleErrorMsg(int code, String errorMsg) {
        if (mOnErrorListener != null) {
            mOnErrorListener.onError(code, errorMsg);
        }
        stop();
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

    interface GAudioTrack {
        void openAudioTrack(int sampleRate, int sampleFormat, int channels, int bytesPerSample);

        int write(@NonNull ByteBuffer buffer, int size);

        void stopAudioTrack();
    }

    // 创建GPlayer.c实例
    private native long nInit(int flag, GPlayer player);

    // 设置surface
    private native void nSetSurface(long nativePlayer, Surface surface);

    // 设置audio render
    private native void nSetAudioTrack(long nativePlayer, GAudioTrack audioTrack);

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

    // 设置播放器属性
    private native void nSetFlags(long nativePlayer, int flags);
}
