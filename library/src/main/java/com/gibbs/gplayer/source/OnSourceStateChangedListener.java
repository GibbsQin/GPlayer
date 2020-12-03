package com.gibbs.gplayer.source;

import com.gibbs.gplayer.GPlayer;

/**
 * media source state listener
 */
public interface OnSourceStateChangedListener {
    void onSourceStateChanged(GPlayer.State state);
}
