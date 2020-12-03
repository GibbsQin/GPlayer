package com.gibbs.gplayer.source;

public interface MediaSourceControl {
    /**
     * set the media buffer callback
     *
     * @param listener callback
     */
    void setOnSourceSizeChangedListener(OnSourceSizeChangedListener listener);

    /**
     * set timestamp listener
     *
     * @param listener listener
     */
    void setOnTimeChangedListener(OnTimeChangedListener listener);

    /**
     * set error listener
     *
     * @param listener listener
     */
    void setOnErrorListener(OnErrorListener listener);
}
