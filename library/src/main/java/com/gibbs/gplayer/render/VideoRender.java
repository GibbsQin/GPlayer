package com.gibbs.gplayer.render;

import android.opengl.GLSurfaceView;

import com.gibbs.gplayer.source.MediaSource;

public interface VideoRender extends GLSurfaceView.Renderer {
    void init(MediaSource mediaSource);

    void render();

    void release();

    void updateMvp(float[] mvp);

    boolean isAvailable();
}
