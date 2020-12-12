package com.gibbs.gplayer;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;

import com.gibbs.gplayer.listener.OnErrorListener;
import com.gibbs.gplayer.listener.OnBufferChangedListener;
import com.gibbs.gplayer.listener.OnPreparedListener;
import com.gibbs.gplayer.listener.OnStateChangedListener;
import com.gibbs.gplayer.listener.OnPositionChangedListener;
import com.gibbs.gplayer.utils.LogUtils;

public class GPlayerView extends GLSurfaceView implements IGPlayer, OnStateChangedListener, View.OnClickListener {
    private static final String TAG = "GPlayerView";

    private GPlayer mGPlayer;
    private OnStateChangedListener mOnStateChangedListener;
    private boolean mHasSetRender = false;

    public GPlayerView(Context context) {
        this(context, null);
    }

    public GPlayerView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setOnClickListener(this);
        initGPlayer();
    }

    private void initGPlayer() {
        if (mGPlayer != null) {
            LogUtils.e("GPlayerView", "initGPlayer has been init");
            return;
        }
        mGPlayer = new GPlayer();
        mGPlayer.setSurface(this);
        mGPlayer.setOnStateChangedListener(this);
    }

    private void destroyGPlayer() {
        mGPlayer.release();
    }

    @Override
    public void setRenderer(Renderer renderer) {
        super.setRenderer(renderer);
        mHasSetRender = true;
    }

    @Override
    public void onResume() {
        if (mHasSetRender) {
            super.onResume();
        }
    }

    @Override
    public void onPause() {
        if (mHasSetRender) {
            super.onPause();
        }
    }

    @Override
    public void setSurface(SurfaceView view) {

    }

    @Override
    public void setDataSource(String url) {
        mGPlayer.setDataSource(url);
    }

    @Override
    public void prepare() {
        mGPlayer.prepare();
    }

    @Override
    public void start() {
        mGPlayer.start();
    }

    @Override
    public void stop() {
        mGPlayer.stop();
    }

    @Override
    public void pause() {
        mGPlayer.pause();
    }

    @Override
    public void release() {
        mGPlayer.release();
    }

    @Override
    public void seekTo(int secondMs) {
        mGPlayer.seekTo(secondMs);
    }

    @Override
    public boolean isPlaying() {
        return mGPlayer.isPlaying();
    }

    @Override
    public void setFlags(int flags) {
        mGPlayer.setFlags(flags);
    }

    @Override
    public int getCurrentPosition() {
        return mGPlayer.getCurrentPosition();
    }

    @Override
    public int getDuration() {
        return mGPlayer.getDuration();
    }

    @Override
    public int getVideoWidth() {
        return mGPlayer.getVideoWidth();
    }

    @Override
    public int getVideoHeight() {
        return mGPlayer.getVideoHeight();
    }

    @Override
    public int getVideoRotate() {
        return mGPlayer.getVideoRotate();
    }

    @Override
    public GPlayer.State getState() {
        return mGPlayer.getState();
    }

    @Override
    public void setOnPreparedListener(OnPreparedListener listener) {
        mGPlayer.setOnPreparedListener(listener);
    }

    @Override
    public void setOnErrorListener(OnErrorListener listener) {
        mGPlayer.setOnErrorListener(listener);
    }

    /**
     * set player state callback
     *
     * @param listener callback
     */
    @Override
    public void setOnStateChangedListener(OnStateChangedListener listener) {
        mOnStateChangedListener = listener;
    }

    @Override
    public void setOnPositionChangedListener(OnPositionChangedListener listener) {
        mGPlayer.setOnPositionChangedListener(listener);
    }

    @Override
    public void setOnBufferChangedListener(OnBufferChangedListener listener) {
        mGPlayer.setOnBufferChangedListener(listener);
    }

    @Override
    public void onClick(View v) {
        if (getState() == GPlayer.State.PAUSED) {
            start();
        } else if (getState() == GPlayer.State.PLAYING) {
            pause();
        }
    }

    @Override
    public void onStateChanged(GPlayer.State state) {
        if (mOnStateChangedListener != null) {
            mOnStateChangedListener.onStateChanged(state);
        }
        if (state == GPlayer.State.PLAYING) {
            post(new Runnable() {
                @Override
                public void run() {
                    resize();
                }
            });
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        LogUtils.i(TAG, "surfaceDestroyed");
        super.surfaceDestroyed(holder);
        destroyGPlayer();
    }

    private void resize() {
        int width = getVideoWidth();
        int height = getVideoHeight();
        int rotate = getVideoRotate();
        ViewGroup.LayoutParams params = getLayoutParams();
        params.width = getWidth();
        if (rotate == 90 || rotate == 270) {
            params.height = (int) (params.width * (width * 1.0f / height));
        } else {
            params.height = (int) (params.width * (height * 1.0f / width));
        }
        setLayoutParams(params);
        LogUtils.i(TAG, "resize to " + params.width + " " + params.height);
    }
}
