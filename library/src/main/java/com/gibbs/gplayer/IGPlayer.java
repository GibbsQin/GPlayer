package com.gibbs.gplayer;

import android.opengl.GLSurfaceView;

import com.gibbs.gplayer.listener.OnBufferChangedListener;
import com.gibbs.gplayer.listener.OnErrorListener;
import com.gibbs.gplayer.listener.OnPositionChangedListener;
import com.gibbs.gplayer.listener.OnPreparedListener;
import com.gibbs.gplayer.listener.OnStateChangedListener;

public interface IGPlayer {
    void setSurface(GLSurfaceView view);

    void setDataSource(String url);

    void prepare();

    void start();

    void stop();

    void pause();

    boolean isPlaying();

    void seekTo(int secondMs);

    int getCurrentPosition();

    int getDuration();

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
