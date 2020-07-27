package com.gibbs.gplayer.render;

import android.opengl.GLSurfaceView;

import com.gibbs.gplayer.media.MediaInfo;

public interface VideoRender extends GLSurfaceView.Renderer {
    void init(MediaInfo mediaInfo);

    void render();

    void release();

    void setAVSync(AVSync avSync);

    void updateMvp(float[] mvp);

    boolean isAvailable();

    void setOnVideoRenderChangedListener(OnVideoRenderChangedListener listener);

    interface OnVideoRenderChangedListener {
        void onSurfaceCreated();

        void onSurfaceChanged(int width, int height);
    }
}
