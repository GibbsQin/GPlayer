package com.gibbs.gplayer.source;

/**
 * playing timestamp
 */
public interface OnTimeChangedListener {
    /**
     * audio timestamp changed
     *
     * @param timeUs timestamp
     */
    void onAudioTimeChanged(long timeUs);

    /**
     * video timestamp changed
     *
     * @param timeUs timestamp
     */
    void onVideoTimeChanged(long timeUs);
}
