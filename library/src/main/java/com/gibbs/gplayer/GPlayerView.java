package com.gibbs.gplayer;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.SurfaceView;
import android.view.ViewGroup;

import com.gibbs.gplayer.listener.OnErrorListener;
import com.gibbs.gplayer.listener.OnBufferChangedListener;
import com.gibbs.gplayer.listener.OnPreparedListener;
import com.gibbs.gplayer.listener.OnStateChangedListener;
import com.gibbs.gplayer.listener.OnPositionChangedListener;
import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.render.GestureGLSurfaceView;
import com.gibbs.gplayer.utils.LogUtils;

public class GPlayerView extends GestureGLSurfaceView implements IGPlayer, OnStateChangedListener {
    private static final String TAG = "GPlayerView";

    private GPlayer mGPlayer;
    private OnStateChangedListener mOnStateChangedListener;
    private boolean mHasSetRender = false;

    public GPlayerView(Context context) {
        super(context);
        initGPlayer();
    }

    public GPlayerView(Context context, AttributeSet attrs) {
        super(context, attrs);
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
    public boolean isPlaying() {
        return mGPlayer.isPlaying();
    }

    @Override
    public void seekTo(int secondMs) {
        mGPlayer.seekTo(secondMs);
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

    private void resize() {
        MediaInfo mediaInfo = mGPlayer.getMediaInfo();
        int width = mediaInfo.getInteger(MediaInfo.KEY_WIDTH, 16);
        int height = mediaInfo.getInteger(MediaInfo.KEY_HEIGHT, 9);
        int rotate = mediaInfo.getInteger(MediaInfo.KEY_VIDEO_ROTATE, 0);
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
