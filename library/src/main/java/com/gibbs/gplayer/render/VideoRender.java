package com.gibbs.gplayer.render;

import android.opengl.GLSurfaceView;

import com.gibbs.gplayer.source.MediaSource;

public interface VideoRender extends GLSurfaceView.Renderer {
    void init(MediaSource mediaSource);

    long render();

    void release();
}
