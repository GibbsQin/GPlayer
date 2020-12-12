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

    void seekTo(int secondMs);

    boolean isPlaying();

    void setFlags(int flags);

    int getCurrentPosition();

    int getDuration();

    int getVideoWidth();

    int getVideoHeight();

    int getVideoRotate();

    GPlayer.State getState();

    void setOnPreparedListener(OnPreparedListener listener);

    void setOnErrorListener(OnErrorListener listener);

    void setOnStateChangedListener(OnStateChangedListener listener);

    void setOnPositionChangedListener(OnPositionChangedListener listener);

    void setOnBufferChangedListener(OnBufferChangedListener listener);
}
