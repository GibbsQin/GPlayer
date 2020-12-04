package com.gibbs.gplayer.listener;

/**
 * playing timestamp
 */
public interface OnPositionChangedListener {
    /**
     * video timestamp changed
     *
     * @param timeUs timestamp
     */
    void onPositionChanged(long timeUs);
}
