package com.gibbs.gplayer;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.text.TextUtils;

import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.render.AudioRender;
import com.gibbs.gplayer.render.EncodeAudioRender;
import com.gibbs.gplayer.render.AVSync;
import com.gibbs.gplayer.render.PcmAudioRender;
import com.gibbs.gplayer.render.SurfaceGLRenderer;
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
        IDLE, PREPARING, PLAYING, FINISHING
    }

    private long mChannelId;

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
        this(view, new MediaSourceImp(url));
    }

    public GPlayer(GLSurfaceView view, String url, boolean decode) {
        this(view, new MediaSourceImp(url, decode));
    }

    public GPlayer(GLSurfaceView view, String url, boolean decode, boolean mediaCodec) {
        this(view, new MediaSourceImp(url, decode, mediaCodec));
    }

    public GPlayer(GLSurfaceView view, MediaSource source) {
        LogUtils.i(TAG, "CoreFlow : new GPlayer");
        mGLSurfaceView = view;
        mContext = mGLSurfaceView.getContext();
        mMediaSource = source;
        mMediaSource.setOnSourceStateChangedListener(this);
        mAudioRender = createAudioRender(mMediaSource);
        mVideoRender = createVideoRender(mGLSurfaceView, mMediaSource);
        mVideoRender.setOnVideoRenderChangedListener(this);
        mVideoRender.setAVSync(this);
        mGLSurfaceView.setEGLContextClientVersion(2);
        mGLSurfaceView.setRenderer(mVideoRender);
        if (mVideoRender instanceof YUVGLRenderer) {
            mGLSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        }
    }

    private AudioRender createAudioRender(MediaSource source) {
        if ((source.getFlag() & MediaSource.FLAG_DECODE) == MediaSource.FLAG_DECODE) {
            return new PcmAudioRender(mMediaSource);
        } else {
            return new EncodeAudioRender(source);
        }
    }

    private VideoRender createVideoRender(GLSurfaceView view, MediaSource source) {
        if ((source.getFlag() & MediaSource.FLAG_DECODE) == MediaSource.FLAG_DECODE) {
            return new YUVGLRenderer(view, source);
        } else {
            return new SurfaceGLRenderer(view, source);
        }
    }

    @Override
    public void onSourceStateChanged(MediaSource.SourceState sourceState) {
        LogUtils.i(TAG, "onSourceStateChanged " + sourceState);
        switch (sourceState) {
            case Init:
                onInit();
                break;
            case Ready:
                break;
            case Release:
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
        if (TextUtils.isEmpty(mMediaSource.getUrl())) {
            return;
        }
        if (mVideoRender != null && mVideoRender.isAvailable()) {
            LogUtils.e(TAG, "preparing");
            setPlayState(State.PREPARING);
            initAVSource(mMediaSource);
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
        mMediaSource.onFinishing();
        nFinish(mChannelId, force);
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
        destroyAVSource();
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
     * get the media info
     *
     * @return media info
     */
    @Override
    public MediaInfo getMediaInfo() {
        return mMediaSource.getMediaInfo();
    }

    /**
     * get player url
     *
     * @return url
     */
    @Override
    public String getUrl() {
        return mMediaSource.getUrl();
    }

    /**
     * set the file url(only support local file now)
     *
     * @param url  file path
     */
    @Override
    public void setUrl(String url) {
        mMediaSource.setUrl(url);
    }

    /**
     * set media source flag
     *
     * @param flag refer to com.gibbs.gplayer.source.MediaSource#FLAG_
     */
    @Override
    public void addFlag(int flag) {
        mMediaSource.addFlag(flag);
    }

    /**
     * get media source flag
     *
     * @return flag
     */
    @Override
    public int getFlag() {
        return mMediaSource.getFlag();
    }

    /**
     * set the media buffer callback
     *
     * @param listener callback
     */
    @Override
    public void setOnSourceSizeChangedListener(OnSourceSizeChangedListener listener) {
        mMediaSource.setOnSourceSizeChangedListener(listener);
    }

    /**
     * set timestamp listener
     *
     * @param listener listener
     */
    @Override
    public void setOnTimeChangedListener(OnTimeChangedListener listener) {
        mMediaSource.setOnTimeChangedListener(listener);
    }

    /**
     * set error listener
     *
     * @param listener listener
     */
    @Override
    public void setOnErrorListener(OnErrorListener listener) {
        mMediaSource.setOnErrorListener(listener);
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
            mMediaSource.clearAudioQueue();
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
            mMediaSource.clearVideoQueue();
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

    private void initAVSource(MediaSource MediaSource) {
        mChannelId = nInitAVSource(MediaSource);
    }

    private void destroyAVSource() {
        LogUtils.i(TAG, "destroyAVSource channelId " + mChannelId);
        nDestroyAVSource(mChannelId);
    }

    private native long nInitAVSource(MediaSource MediaSource);

    private native void nFinish(long channelId, boolean force);

    private native void nDestroyAVSource(long channelId);
}
