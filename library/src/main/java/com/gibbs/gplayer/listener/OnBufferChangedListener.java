package com.gibbs.gplayer.listener;

/**
 * media buffer size listener
 */
public interface OnBufferChangedListener {
    /**
     * audio decoded frame size changed
     *
     * @param size frame size
     */
    void onAudioFrameSizeChanged(int size);

    /**
     * video decoded frame size changed
     *
     * @param size frame size
     */
    void onVideoFrameSizeChanged(int size);

    /**
     * audio compressed frame size changed
     *
     * @param size frame size
     */
    void onAudioPacketSizeChanged(int size);

    /**
     * video compressed frame size changed
     *
     * @param size frame size
     */
    void onVideoPacketSizeChanged(int size);
}
