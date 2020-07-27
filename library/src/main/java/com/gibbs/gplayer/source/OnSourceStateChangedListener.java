package com.gibbs.gplayer.source;

/**
 * media source state listener
 */
public interface OnSourceStateChangedListener {
    /**
     * Init : has received media head info
     * Ready : playing
     * Release : media source was release
     * Error : error occur
     */
    enum SourceState {
        Init, Ready, Release, Error
    }

    void onSourceStateChanged(SourceState sourceState);
}
