package com.gibbs.gplayer;

import android.content.Context;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

public class GPlayerView extends SurfaceView implements SurfaceHolder.Callback, View.OnClickListener {
    private GPlayer mGPlayer;

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
        getHolder().addCallback(this);
    }

    public void setDataSource(String url) {
        mGPlayer.setDataSource(url);
    }

    public void prepare() {
        mGPlayer.prepare();
    }

    public void start() {
        mGPlayer.start();
    }

    public void stop() {
        mGPlayer.stop();
    }

    public void pause() {
        mGPlayer.pause();
    }

    public void release() {
        mGPlayer.release();
    }

    public void seekTo(int secondMs) {
        mGPlayer.seekTo(secondMs);
    }

    public boolean isPlaying() {
        return mGPlayer.isPlaying();
    }

    public void setFlags(int flags) {
        mGPlayer.setFlags(flags);
    }

    public int getCurrentPosition() {
        return mGPlayer.getCurrentPosition();
    }

    public GPlayer.State getState() {
        return mGPlayer.getState();
    }

    public void setOnPreparedListener(GPlayer.OnPreparedListener listener) {
        mGPlayer.setOnPreparedListener(listener);
    }

    public void setOnErrorListener(GPlayer.OnErrorListener listener) {
        mGPlayer.setOnErrorListener(listener);
    }

    public void setOnStateChangedListener(GPlayer.OnStateChangedListener listener) {
        mGPlayer.setOnStateChangedListener(listener);
    }

    public void setOnPositionChangedListener(GPlayer.OnPositionChangedListener listener) {
        mGPlayer.setOnPositionChangedListener(listener);
    }

    public void setOnBufferChangedListener(GPlayer.OnBufferChangedListener listener) {
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
    public void surfaceCreated(SurfaceHolder holder) {
        mGPlayer.setSurface(holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        mGPlayer.setSurface(null);
        mGPlayer.stop();
        mGPlayer.release();
    }
}
