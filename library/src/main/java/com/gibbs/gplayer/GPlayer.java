package com.gibbs.gplayer;

import android.content.Context;
import android.opengl.GLSurfaceView;

import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.render.AudioRender;
import com.gibbs.gplayer.render.AVSync;
import com.gibbs.gplayer.render.PcmAudioRender;
import com.gibbs.gplayer.render.VideoFrameReleaseTimeHelper;
import com.gibbs.gplayer.render.VideoRender;
import com.gibbs.gplayer.render.YUVGLRenderer;
import com.gibbs.gplayer.source.MediaSource;
import com.gibbs.gplayer.source.MediaSourceControl;
import com.gibbs.gplayer.source.MediaSourceImp;
import com.gibbs.gplayer.source.OnErrorListener;
import com.gibbs.gplayer.source.OnSourceSizeChangedListener;
import com.gibbs.gplayer.source.OnSourceStateChangedListener;
import com.gibbs.gplayer.source.OnTimeChangedListener;
import com.gibbs.gplayer.utils.LogUtils;

public class GPlayer implements MediaSourceControl, OnSourceStateChangedListener, AVSync,
        VideoRender.OnVideoRenderChangedListener {
    private static final String TAG = "GPlayerJ";

    static {
        System.loadLibrary("gplayer");
    }

    public enum State {
        IDLE, PREPARING, PLAYING, FINISHING, RELEASED
    }

    private int mChannelId = hashCode();
    private String mUrl;

    private Context mContext;
    private GLSurfaceView mGLSurfaceView;

    private AudioRender mAudioRender;
    private VideoRender mVideoRender;

    //AVSync
    private VideoFrameReleaseTimeHelper mFrameReleaseTimeHelper;
    private long mDeltaTimeUs;

    private MediaSource mMediaSource;
    private AudioPlayThread mAudioPlayThread;
    private VideoPlayThread mVideoPlayThread;
    private boolean mIsProcessingSource;
    private boolean mIsAudioPlaying;
    private boolean mIsVideoPlaying;
    private boolean mStartPlayWhenReady;
    private State mPlayState = State.IDLE;

    private PlayStateChangedListener mPlayStateChangedListener;

    public GPlayer(GLSurfaceView view, String url) {
        this(view, url, false);
    }

    public GPlayer(GLSurfaceView view, String url, boolean mediaCodec) {
        LogUtils.i(TAG, "CoreFlow : new GPlayer");
        mGLSurfaceView = view;
        mContext = mGLSurfaceView.getContext();
        mMediaSource = new MediaSourceImp(mChannelId);
        mAudioRender = createAudioRender(mMediaSource);
        mVideoRender = createVideoRender(mGLSurfaceView, mMediaSource);
        mVideoRender.setOnVideoRenderChangedListener(this);
        mVideoRender.setAVSync(this);
        mGLSurfaceView.setEGLContextClientVersion(2);
        mGLSurfaceView.setRenderer(mVideoRender);
        if (mVideoRender instanceof YUVGLRenderer) {
            mGLSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        }
        mUrl = url;
        init(url, mediaCodec ? 2 : 0);
    }

    private AudioRender createAudioRender(MediaSource source) {
        return new PcmAudioRender(source);
    }

    private VideoRender createVideoRender(GLSurfaceView view, MediaSource source) {
        return new YUVGLRenderer(view, source);
    }

    @Override
    public void onSourceStateChanged(State sourceState) {
        LogUtils.i(TAG, "onSourceStateChanged " + sourceState);
        switch (sourceState) {
            case PREPARING:
                onInit();
                break;
            case RELEASED:
                onRelease();
                break;
        }
    }

    /**
     * is this player playing
     *
     * @return true playing, false not playing
     */
    public boolean isPlaying() {
        return mPlayState == State.PLAYING;
    }

    /**
     * start to play
     */
    public void prepare() {
        if (mPlayState != State.IDLE) {
            return;
        }
        if (mVideoRender != null && mVideoRender.isAvailable()) {
            LogUtils.e(TAG, "preparing");
            setPlayState(State.PREPARING);
            nStart(mChannelId);
        } else {
            LogUtils.i(TAG, "waiting for GLSurfaceView ready");
            mStartPlayWhenReady = true;
        }
    }

    /**
     * stop play
     */
    public void finish() {
        finish(true);
    }

    /**
     * stop play
     */
    public void finish(boolean force) {
        LogUtils.i(TAG, "CoreFlow finishing channelId " + mChannelId);
        if (!isPlaying() && !force) {
            return;
        }
        setPlayState(State.FINISHING);
        nStop(mChannelId, force);
    }

    private void onInit() {
        LogUtils.i(TAG, "----------onInit----------");
        mIsProcessingSource = true;
        mAudioRender.init(mMediaSource.getMediaInfo());
        mVideoRender.init(mMediaSource.getMediaInfo());

        mAudioPlayThread = new AudioPlayThread();
        mAudioPlayThread.start();
        mVideoPlayThread = new VideoPlayThread();
        mVideoPlayThread.start();
    }

    private void onRelease() {
        LogUtils.e(TAG, "----------onRelease----------");
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
        LogUtils.e(TAG, "CoreFlow : finished");
        destroy();
    }

    public MediaInfo getMediaInfo() {
        return mMediaSource.getMediaInfo();
    }

    public String getUrl() {
        return mUrl;
    }

    /**
     * set player state callback
     *
     * @param listener callback
     */
    public void setPlayStateChangedListener(PlayStateChangedListener listener) {
        mPlayStateChangedListener = listener;
    }

    /**
     * set the media buffer callback
     *
     * @param listener callback
     */
    @Override
    public void setOnSourceSizeChangedListener(OnSourceSizeChangedListener listener) {
    }

    /**
     * set timestamp listener
     *
     * @param listener listener
     */
    @Override
    public void setOnTimeChangedListener(OnTimeChangedListener listener) {
    }

    /**
     * set error listener
     *
     * @param listener listener
     */
    @Override
    public void setOnErrorListener(OnErrorListener listener) {
    }

    @Override
    public long getNowUs() {
        //the timestamp of audio playing
        if (mAudioRender == null) {
            return System.nanoTime() / 1000;
        }

        return mAudioRender.getAudioTimeUs();
    }

    @Override
    public long getRealTimeUsForMediaTime(long mediaTimeUs) {
        long nowUs = getNowUs();
        if (mDeltaTimeUs == -1) {
            mDeltaTimeUs = nowUs - mediaTimeUs;
        }
        long earlyUs = mDeltaTimeUs + mediaTimeUs - nowUs;
        long unadjustedFrameReleaseTimeNs = System.nanoTime() + (earlyUs * 1000);
        long adjustedReleaseTimeNs = mFrameReleaseTimeHelper.adjustReleaseTime(
                mDeltaTimeUs + mediaTimeUs, unadjustedFrameReleaseTimeNs);
        return adjustedReleaseTimeNs / 1000;
    }

    @Override
    public long getVsyncDurationNs() {
        if (mFrameReleaseTimeHelper != null) {
            return mFrameReleaseTimeHelper.getVsyncDurationNs();
        } else {
            return -1;
        }
    }

    @Override
    public void onSurfaceCreated() {
        LogUtils.i(TAG, "onSurfaceCreated");
        if (mStartPlayWhenReady) {
            prepare();
            mStartPlayWhenReady = false;
        }
    }

    @Override
    public void onSurfaceChanged(int width, int height) {
        LogUtils.i(TAG, "onSurfaceChanged");
    }

    private void setPlayState(State state) {
        synchronized (this) {
            if (state == mPlayState) {
                return;
            }
            mPlayState = state;
        }
        LogUtils.e(TAG, "CoreFlow setPlayState state = " + state);
        if (mPlayState == State.IDLE) {
            mMediaSource.flushBuffer();
        }
        if (mPlayStateChangedListener != null) {
            mPlayStateChangedListener.onPlayStateChanged(mPlayState);
        }
    }

    private class AudioPlayThread extends Thread {
        @Override
        public void run() {
            super.run();
            mIsAudioPlaying = true;
            LogUtils.i(TAG, "AudioPlayThread init " + getId());
            setPlayState(GPlayer.State.PLAYING);
            while (mIsProcessingSource) {
                mAudioRender.render();
            }
            mAudioRender.release();
            mIsAudioPlaying = false;
            LogUtils.i(TAG, "AudioPlayThread end " + getId());
            if (!mIsAudioPlaying && !mIsVideoPlaying) {
                setPlayState(GPlayer.State.IDLE);
            }
        }
    }

    private class VideoPlayThread extends Thread {
        @Override
        public void run() {
            super.run();
            LogUtils.i(TAG, "VideoPlayThread init " + getId());
            mIsVideoPlaying = true;
            setPlayState(GPlayer.State.PLAYING);
            mFrameReleaseTimeHelper = new VideoFrameReleaseTimeHelper(mContext);
            mFrameReleaseTimeHelper.enable();
            mDeltaTimeUs = -1;
            while (mIsProcessingSource) {
                mVideoRender.render();
            }
            mVideoRender.release();
            mFrameReleaseTimeHelper.disable();
            mFrameReleaseTimeHelper = null;
            mIsVideoPlaying = false;
            LogUtils.i(TAG, "VideoPlayThread end " + getId());
            if (!mIsAudioPlaying && !mIsVideoPlaying) {
                setPlayState(GPlayer.State.IDLE);
            }
        }
    }

    public interface PlayStateChangedListener {
        void onPlayStateChanged(State state);
    }

    private void init(String url, int flag) {
        nInit(mChannelId, flag, url, this);
    }

    private void destroy() {
        LogUtils.i(TAG, "destroy channelId " + mChannelId);
        nDestroy(mChannelId);
    }

    //call by jni
    public void onMessageCallback(int what, int arg1, int arg2, String msg1, String msg2, Object object) {

    }

    private native void nInit(int channelId, int flag, String url, GPlayer player);

    private native void nStart(long channelId);

    private native void nStop(long channelId, boolean force);

    private native void nDestroy(long channelId);
}
