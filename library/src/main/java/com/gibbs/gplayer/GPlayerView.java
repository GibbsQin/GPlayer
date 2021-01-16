/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

package com.gibbs.gplayer;

import android.content.Context;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class GPlayerView extends SurfaceView implements SurfaceHolder.Callback {
    private GPlayer mGPlayer;

    public GPlayerView(Context context) {
        this(context, null);
    }

    public GPlayerView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initGPlayer();
    }

    private void initGPlayer() {
        if (mGPlayer != null) {
            return;
        }
        mGPlayer = new GPlayer();
        getHolder().addCallback(this);
    }

    public void setDataSource(String url) {
        mGPlayer.setDataSource(url);
    }

    public void prepareAsync() {
        mGPlayer.prepareAsync();
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

    public void reset() {
        mGPlayer.reset();
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

    public int getDuration() {
        return mGPlayer.getDuration();
    }

    public int getVideoWidth() {
        return mGPlayer.getVideoWidth();
    }

    public int getVideoHeight() {
        return mGPlayer.getVideoHeight();
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

    public void setOnSeekStateChangedListener(GPlayer.OnSeekStateChangedListener listener) {
        mGPlayer.setOnSeekStateChangedListener(listener);
    }

    public void setOnCompletedListener(GPlayer.OnCompletedListener listener) {
        mGPlayer.setOnCompletedListener(listener);
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
