package com.gibbs.gplayer.codec;

import android.view.Surface;

import com.gibbs.gplayer.render.AVSync;

public interface VideoDecoder extends BaseDecoder {
    void setSurface(Surface surface);

    void setAVSync(AVSync avSync);
}
