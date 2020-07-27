package com.gibbs.gplayer.source;

import com.gibbs.gplayer.media.MediaInfo;

public interface MediaSourceControl {

    /**
     * get the media info
     *
     * @return media info
     */
    MediaInfo getMediaInfo();

    /**
     * get player url
     *
     * @return url
     */
    String getUrl();

    /**
     * set the file url(only support local file now)
     *
     * @param type refer to com.gibbs.gplayer.source.MediaSource#SOURCE_TYPE_
     * @param url  file path
     */
    void setUrl(int type, String url);

    /**
     * set media source flag
     *
     * @param flag refer to com.gibbs.gplayer.source.MediaSource#FLAG_
     */
    void addFlag(int flag);

    /**
     * get media source flag
     *
     * @return flag
     */
    int getFlag();

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
