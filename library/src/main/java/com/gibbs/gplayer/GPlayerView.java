package com.gibbs.gplayer;

import android.content.Context;
import android.util.AttributeSet;

import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.render.GestureGLSurfaceView;
import com.gibbs.gplayer.source.MediaSourceControl;
import com.gibbs.gplayer.source.OnErrorListener;
import com.gibbs.gplayer.source.OnSourceSizeChangedListener;
import com.gibbs.gplayer.source.OnTimeChangedListener;
import com.gibbs.gplayer.utils.LogUtils;

public class GPlayerView extends GestureGLSurfaceView implements MediaSourceControl {
    private GPlayer mGPlayer;

    public GPlayerView(Context context) {
        super(context);
    }

    public GPlayerView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    private void initGPlayer(int type, String url, boolean decode, boolean mediaCodec) {
        if (mGPlayer != null) {
            LogUtils.e("GPlayerView", "initGPlayer has been init");
            return;
        }
        mGPlayer = new GPlayer(this, type, url, decode, mediaCodec);
    }

    /**
     * start to play
     */
    public void startPlay() {
        mGPlayer.prepare();
    }

    /**
     * stop play
     */
    public void stopPlay() {
        mGPlayer.finish(true);
    }

    /**
     * set player state callback
     *
     * @param listener callback
     */
    public void setPlayStateChangedListener(GPlayer.PlayStateChangedListener listener) {
        mGPlayer.setPlayStateChangedListener(listener);
    }

    /**
     * get the media info
     *
     * @return media info
     */
    @Override
    public MediaInfo getMediaInfo() {
        return mGPlayer.getMediaInfo();
    }

    /**
     * get player url
     *
     * @return url
     */
    @Override
    public String getUrl() {
        return mGPlayer.getUrl();
    }

    /**
     * set the file url(only support local file now)
     *
     * @param type refer to com.gibbs.gplayer.source.MediaSource#SOURCE_TYPE_
     * @param url  file path
     */
    @Override
    public void setUrl(int type, String url) {
        setUrl(type, url, true, false);
    }

    /**
     * set the file url(only support local file now)
     *
     * @param type       refer to com.gibbs.gplayer.source.MediaSource#SOURCE_TYPE_
     * @param url        file path
     * @param decode     true : decode the packet on jni layer，false : decode the packet on java layer
     * @param mediaCodec true : use mediacodec，false : use ffmpeg. valid when decode is true
     */
    public void setUrl(int type, String url, boolean decode, boolean mediaCodec) {
        initGPlayer(type, url, decode, mediaCodec);
    }

    /**
     * set media source flag
     *
     * @param flag refer to com.gibbs.gplayer.source.MediaSource#FLAG_
     */
    @Override
    public void addFlag(int flag) {
        mGPlayer.addFlag(flag);
    }

    /**
     * get media source flag
     *
     * @return flag
     */
    @Override
    public int getFlag() {
        return mGPlayer.getFlag();
    }

    /**
     * set the media buffer callback
     *
     * @param listener callback
     */
    @Override
    public void setOnSourceSizeChangedListener(OnSourceSizeChangedListener listener) {
        mGPlayer.setOnSourceSizeChangedListener(listener);
    }

    /**
     * set timestamp listener
     *
     * @param listener listener
     */
    @Override
    public void setOnTimeChangedListener(OnTimeChangedListener listener) {
        mGPlayer.setOnTimeChangedListener(listener);
    }

    /**
     * set error listener
     *
     * @param listener listener
     */
    @Override
    public void setOnErrorListener(OnErrorListener listener) {
        mGPlayer.setOnErrorListener(listener);
    }
}
