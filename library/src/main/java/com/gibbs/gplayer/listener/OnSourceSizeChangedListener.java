package com.gibbs.gplayer.listener;

/**
 * media buffer size listener
 */
public interface OnSourceSizeChangedListener {
    /**
     * audio decoded frame size changed
     *
     * @param size frame size
     */
    void onLocalAudioSizeChanged(int size);

    /**
     * video decoded frame size changed
     *
     * @param size frame size
     */
    void onLocalVideoSizeChanged(int size);

    /**
     * audio compressed frame size changed
     *
     * @param size frame size
     */
    void onRemoteAudioSizeChanged(int size);

    /**
     * video compressed frame size changed
     *
     * @param size frame size
     */
    void onRemoteVideoSizeChanged(int size);
}
