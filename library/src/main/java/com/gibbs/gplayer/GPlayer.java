/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

package com.gibbs.gplayer;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;
import android.util.ArrayMap;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

import androidx.annotation.NonNull;

import java.nio.ByteBuffer;
import java.util.Map;

public class GPlayer {
    private static final String TAG = "GPlayerJ";

    /**
     * ffmpeg sample fmt
     */
    private static final int FFMPEG_SAMPLE_FMT_U8 = 0;          ///< unsigned 8 bits
    private static final int FFMPEG_SAMPLE_FMT_S16 = 1;         ///< signed 16 bits
    private static final int FFMPEG_SAMPLE_FMT_S32 = 2;         ///< signed 32 bits
    private static final int FFMPEG_SAMPLE_FMT_FLT = 3;         ///< float
    private static final int FFMPEG_SAMPLE_FMT_DBL = 4;         ///< double
    private static final int FFMPEG_SAMPLE_FMT_U8P = 5;         ///< unsigned 8 bits, planar
    private static final int FFMPEG_SAMPLE_FMT_S16P = 6;        ///< signed 16 bits, planar
    private static final int FFMPEG_SAMPLE_FMT_S32P = 7;        ///< signed 32 bits, planar
    private static final int FFMPEG_SAMPLE_FMT_FLTP = 8;        ///< float, planar
    private static final int FFMPEG_SAMPLE_FMT_DBLP = 9;        ///< double, planar
    private static final int FFMPEG_SAMPLE_FMT_S64 = 10;        ///< signed 64 bits
    private static final int FFMPEG_SAMPLE_FMT_S64P = 11;       ///< signed 64 bits, planar

    private static final Map<Integer, Integer> sSampleFmtMap = new ArrayMap<>();
    private static final Map<Integer, Integer> sChannelFmtMap = new ArrayMap<>();

    static {
        sChannelFmtMap.put(1, AudioFormat.CHANNEL_OUT_MONO);
        sChannelFmtMap.put(2, AudioFormat.CHANNEL_OUT_STEREO);
        sChannelFmtMap.put(3, AudioFormat.CHANNEL_OUT_SURROUND);
        sChannelFmtMap.put(4, AudioFormat.CHANNEL_OUT_QUAD);
        sChannelFmtMap.put(6, AudioFormat.CHANNEL_OUT_5POINT1);

        sSampleFmtMap.put(FFMPEG_SAMPLE_FMT_U8, AudioFormat.ENCODING_PCM_8BIT);
        sSampleFmtMap.put(FFMPEG_SAMPLE_FMT_S16, AudioFormat.ENCODING_PCM_16BIT);
        sSampleFmtMap.put(FFMPEG_SAMPLE_FMT_FLTP, AudioFormat.ENCODING_PCM_FLOAT);
    }

    private static final int MSG_TYPE_ERROR = 0;
    private static final int MSG_TYPE_STATE = 1;
    private static final int MSG_TYPE_TIME = 2;
    private static final int MSG_TYPE_SIZE = 3;
    private static final int MSG_TYPE_COMPLETE = 6;
    private static final int MSG_TYPE_SEEK = 7;

    private static final int ERROR_DEMUXING = 0;
    private static final int ERROR_DECODING = 1;
    private static final int ERROR_RENDERING = 2;
    private static final int ERROR_SEEK = 3;

    private static final int ERROR_ILLEGAL_STATE = 100;
    private static final int ERROR_INVALID_SOURCE = 101;

    public static final int AV_FLAG_SOURCE_MEDIA_CODEC = 0x00000002;

    public static boolean enableMediaCodec = false;

    static {
        System.loadLibrary("gplayer");
    }

    public enum State {
        IDLE, INITIALIZED, PREPARING, PREPARED, STARTED, PAUSED, STOPPED, COMPLETED, END, ERROR
    }

    private final long mNativePlayer;
    private final Handler mHandler = new Handler(Looper.getMainLooper());
    private GAudioTrack mGAudioTrack;
    private String mUrl;
    private State mPlayState = State.IDLE;
    private State mTargetState;
    private int mCurrentPositionUs;
    private boolean mIsSeeking = false;
    private boolean mIsLooping = false;
    private final Object mStateLock = new Object();
    private final Object mPrepareLock = new Object();

    private OnPreparedListener mOnPreparedListener;
    private OnErrorListener mOnErrorListener;
    private OnStateChangedListener mOnStateChangedListener;
    private OnPositionChangedListener mOnPositionChangedListener;
    private OnBufferChangedListener mOnBufferChangedListener;
    private OnSeekStateChangedListener mOnSeekStateChangedListener;
    private OnCompletedListener mOnCompletedListener;

    public GPlayer() {
        this(enableMediaCodec);
    }

    public GPlayer(boolean mediaCodec) {
        Log.i(TAG, "CoreFlow : mediaCodec " + mediaCodec);
        mNativePlayer = nInit(mediaCodec ? AV_FLAG_SOURCE_MEDIA_CODEC : 0, this);
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
        if (mPlayState != State.IDLE) {
            Log.e(TAG, "CoreFlow : can not setDataSource in " + mPlayState);
            if (mOnErrorListener != null) {
                mOnErrorListener.onError(ERROR_ILLEGAL_STATE, "Illegal state");
            }
            return;
        }
        if (TextUtils.isEmpty(url)) {
            Log.e(TAG, "CoreFlow : invalid data source : " + url);
            if (mOnErrorListener != null) {
                mOnErrorListener.onError(ERROR_INVALID_SOURCE, "Invalid url");
            }
            return;
        }
        mUrl = url;
        setPlayState(State.INITIALIZED);
    }

    public synchronized void prepareAsync() {
        if (mPlayState != State.INITIALIZED && mPlayState != State.STOPPED) {
            Log.e(TAG, "CoreFlow : can not prepareAsync in " + mPlayState);
            if (mOnErrorListener != null) {
                mOnErrorListener.onError(ERROR_ILLEGAL_STATE, "Illegal state");
            }
            return;
        }
        mIsSeeking = false;
        setFlags(enableMediaCodec ? AV_FLAG_SOURCE_MEDIA_CODEC : 0);
        nPrepare(mNativePlayer, mUrl);
    }

    public synchronized void prepare() {
        if (mPlayState != State.INITIALIZED && mPlayState != State.STOPPED) {
            Log.e(TAG, "CoreFlow : can not prepare in " + mPlayState);
            if (mOnErrorListener != null) {
                mOnErrorListener.onError(ERROR_ILLEGAL_STATE, "Illegal state");
            }
            return;
        }
        mIsSeeking = false;
        setPlayState(State.PREPARING);
        setFlags(enableMediaCodec ? AV_FLAG_SOURCE_MEDIA_CODEC : 0);
        nPrepare(mNativePlayer, mUrl);
        synchronized (mPrepareLock) {
            try {
                mPrepareLock.wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        start();
    }

    public synchronized void start() {
        if (mPlayState != State.PREPARED && mPlayState != State.PAUSED) {
            Log.e(TAG, "can not start in " + mPlayState);
            if (mOnErrorListener != null) {
                mOnErrorListener.onError(ERROR_ILLEGAL_STATE, "Illegal state");
            }
            return;
        }
        if (mPlayState == State.PREPARED) {
            if (mGAudioTrack == null) {
                mGAudioTrack = new DefaultGAudioTrack();
            }
            nSetAudioTrack(mNativePlayer, mGAudioTrack);
            nStart(mNativePlayer);
        } else if (mPlayState == State.PAUSED) {
            nResume(mNativePlayer);
            setPlayState(State.STARTED);
        }
    }

    public synchronized void stop() {
        if (!isPlaying()) {
            Log.e(TAG, "CoreFlow : can not stop in " + mPlayState);
            if (mOnErrorListener != null) {
                mOnErrorListener.onError(ERROR_ILLEGAL_STATE, "Illegal state");
            }
            return;
        }
        nStop(mNativePlayer, true);
    }

    public synchronized void pause() {
        if (mPlayState != State.STARTED) {
            Log.e(TAG, "CoreFlow : can not pause in " + mPlayState);
            if (mOnErrorListener != null) {
                mOnErrorListener.onError(ERROR_ILLEGAL_STATE, "Illegal state");
            }
            return;
        }
        nPause(mNativePlayer);
        setPlayState(State.PAUSED);
    }

    public synchronized void release() {
        if (mPlayState == State.END) {
            return;
        }
        if (mPlayState == State.STOPPED) {
            Log.i(TAG, "CoreFlow : release native player " + mNativePlayer);
            nRelease(mNativePlayer);
        } else {
            Log.i(TAG, "CoreFlow : target to release");
            mTargetState = State.END;
            nStop(mNativePlayer, true);
        }
    }

    public synchronized void reset() {
        if (mPlayState == State.IDLE) {
            return;
        }
        Log.i(TAG, "CoreFlow : target to idle");
        mTargetState = State.IDLE;
        nStop(mNativePlayer, true);
    }

    public synchronized void seekTo(int secondMs) {
        if (!isPlaying()) {
            Log.e(TAG, "CoreFlow : can not seek in " + mPlayState);
            if (mOnErrorListener != null) {
                mOnErrorListener.onError(ERROR_ILLEGAL_STATE, "Illegal state");
            }
            return;
        }
        if (mIsSeeking) {
            Log.e(TAG, "CoreFlow : seeking now");
            if (mOnErrorListener != null) {
                mOnErrorListener.onError(ERROR_SEEK, "Seeking now");
            }
            return;
        }
        nSeekTo(mNativePlayer, secondMs);
    }

    public void setLooping(boolean looping) {
        mIsLooping = looping;
    }

    public boolean isPlaying() {
        return mPlayState == State.PREPARED || mPlayState == State.PAUSED || mPlayState == State.STARTED;
    }

    public void setFlags(int flags) {
        nSetFlags(mNativePlayer, flags);
    }

    public int getCurrentPosition() {
        return mCurrentPositionUs;
    }

    public int getDuration() {
        return nGetDuration(mNativePlayer);
    }

    public int getVideoWidth() {
        return nGetVideoWidth(mNativePlayer);
    }

    public int getVideoHeight() {
        return nGetVideoHeight(mNativePlayer);
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

    public interface OnPositionChangedListener {
        void onPositionChanged(int timeMs);
    }

    public void setOnBufferChangedListener(OnBufferChangedListener listener) {
        mOnBufferChangedListener = listener;
    }

    public interface OnBufferChangedListener {
        void onBufferChanged(int percent);
    }

    public interface OnSeekStateChangedListener {
        void onSeekStateChanged(int state);
    }

    public void setOnSeekStateChangedListener(OnSeekStateChangedListener listener) {
        mOnSeekStateChangedListener = listener;
    }

    public interface OnCompletedListener {
        void onCompleted();
    }

    public void setOnCompletedListener(OnCompletedListener listener) {
        mOnCompletedListener = listener;
    }

    public String getUrl() {
        return mUrl;
    }

    private void setPlayState(State state) {
        synchronized (mStateLock) {
            if (mPlayState == state) {
                return;
            }
            Log.e(TAG, "CoreFlow : setPlayState from " + mPlayState + " to " + state);
            mPlayState = state;
        }
        switch (state) {
            case PREPARED:
                onPrepared();
                break;
            case STOPPED:
                onStopped();
                break;
            case END:
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
        if (mTargetState == State.END) {
            release();
        } else if (mTargetState == State.IDLE) {
            setPlayState(State.IDLE);
        } else if (mTargetState == State.STARTED) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    prepare();
                }
            });
        }
        mTargetState = null;
    }

    private void onReleased() {

    }

    //call by jni
    public void onMessageCallback(final int what, final int arg1, final long arg2, final String msg1,
                                  final String msg2, final Object object) {
        Log.d(TAG, "onMessageCallback " + what + ", " + arg1 + ", " + arg2);
        if (mPlayState == State.PREPARING) {
            synchronized (mPrepareLock) {
                mPrepareLock.notifyAll();
            }
        }
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
                        handleSizeMsg(arg1);
                        break;
                    case MSG_TYPE_COMPLETE:
                        handleCompleteMsg();
                        break;
                    case MSG_TYPE_SEEK:
                        handleSeekMsg(arg1);
                        break;
                }
            }
        });
    }

    private void handleErrorMsg(int code, String errorMsg) {
        Log.i(TAG, "CoreFlow : handleErrorMsg " + code + ", " + errorMsg);
        setPlayState(State.ERROR);
        if (mOnErrorListener != null) {
            mOnErrorListener.onError(code, errorMsg);
        }
        if (code == ERROR_SEEK) {
            mIsSeeking = false;
        } else {
            nStop(mNativePlayer, true);
        }
    }

    private void handleStateMsg(int state) {
        State playState = State.values()[state];
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

    private void handleSizeMsg(int state) {
        if (mOnBufferChangedListener != null) {
            if (state == 0) {
                mOnBufferChangedListener.onBufferChanged(0);
            } else {
                mOnBufferChangedListener.onBufferChanged(100);
            }
        }
    }

    private void handleCompleteMsg() {
        Log.i(TAG, "CoreFlow : handleCompleteMsg");
        if (mIsLooping) {
            mTargetState = State.STARTED;
        }
        nStop(mNativePlayer, true);
        if (mOnCompletedListener != null) {
            mOnCompletedListener.onCompleted();
        }
    }

    private void handleSeekMsg(int state) {
        Log.i(TAG, "CoreFlow : handleSeekMsg " + state);
        mIsSeeking = state == 0;
        if (mOnSeekStateChangedListener != null) {
            mOnSeekStateChangedListener.onSeekStateChanged(state);
        }
    }

    interface GAudioTrack {
        void open(int sampleRate, int fmt, int channels, int bytesPerSample);

        int write(@NonNull ByteBuffer buffer, int size);

        void close();
    }

    private static class DefaultGAudioTrack implements GPlayer.GAudioTrack {
        private static final String TAG = "AudioTrackWrap";
        private AudioTrack mAudioTrack;

        @Override
        public void open(int sampleRate, int fmt, int channels, int bytesPerSample) {
            Log.i(TAG, "openAudioTrack " + sampleRate + " " + fmt + " " + channels + " " + bytesPerSample);
            try {
                int sampleFormat = sSampleFmtMap.get(fmt);
                int channelLayout = sChannelFmtMap.get(channels);
                int minBufferSize = AudioTrack.getMinBufferSize(sampleRate, channelLayout, sampleFormat);
                int bufferSizeInBytes = Math.round(minBufferSize * 1.0f / bytesPerSample + 0.5f) * bytesPerSample;
                mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate, channelLayout,
                        sampleFormat, minBufferSize * 2, AudioTrack.MODE_STREAM);
                mAudioTrack.play();
                Log.i(TAG, "openAudioTrack min buffer size:" + minBufferSize + ", bufferSizeInBytes:" + bufferSizeInBytes);
            } catch (Exception e) {
                Log.e(TAG, "create AudioTrack error: " + e.getMessage());
            }
        }

        @Override
        public int write(@NonNull ByteBuffer buffer, int size) {
            return mAudioTrack.write(buffer, size, AudioTrack.WRITE_BLOCKING);
        }

        @Override
        public void close() {
            if (mAudioTrack != null) {
                mAudioTrack.flush();
                mAudioTrack.stop();
                mAudioTrack.release();
                mAudioTrack = null;
                Log.i(TAG, "stopAudioTrack");
            }
        }
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

    // 获取视频时长
    private native int nGetDuration(long nativePlayer);

    // 获取视频宽度
    private native int nGetVideoWidth(long nativePlayer);

    // 获取视频高度
    private native int nGetVideoHeight(long nativePlayer);
}
