package com.gibbs.gplayer.render;

import android.opengl.GLSurfaceView;

import com.gibbs.gplayer.media.MediaInfo;

public interface VideoRender extends GLSurfaceView.Renderer {
    void init(MediaInfo mediaInfo);

    void render();

    void release();

    void updateMvp(float[] mvp);

    boolean isAvailable();
}
