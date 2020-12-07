package com.gibbs.gplayer;

import android.view.SurfaceView;

import com.gibbs.gplayer.listener.OnBufferChangedListener;
import com.gibbs.gplayer.listener.OnErrorListener;
import com.gibbs.gplayer.listener.OnPositionChangedListener;
import com.gibbs.gplayer.listener.OnPreparedListener;
import com.gibbs.gplayer.listener.OnStateChangedListener;

public interface IGPlayer {
    void setSurface(SurfaceView view);

    void setDataSource(String url);

    void prepare();

    void start();

    void stop();

    void pause();

    void release();

    boolean isPlaying();

    void seekTo(int secondMs);

    int getCurrentPosition();

    int getDuration();

    int getVideoWidth();

    int getVideoHeight();

    int getVideoRotate();

    void setOnPreparedListener(OnPreparedListener listener);

    void setOnErrorListener(OnErrorListener listener);

    /**
     * set player state callback
     *
     * @param listener callback
     */
    void setOnStateChangedListener(OnStateChangedListener listener);

    void setOnPositionChangedListener(OnPositionChangedListener listener);

    void setOnBufferChangedListener(OnBufferChangedListener listener);
}
