package com.gibbs.gplayer;

import android.content.Context;
import android.util.AttributeSet;
import android.view.ViewGroup;

import com.gibbs.gplayer.listener.OnStateChangedListener;
import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.render.GestureGLSurfaceView;
import com.gibbs.gplayer.utils.LogUtils;

public class GPlayerView extends GestureGLSurfaceView implements OnStateChangedListener {
    private static final String TAG = "GPlayerView";

    private GPlayer mGPlayer;
    private OnStateChangedListener mPlayStateChangedListener;

    public GPlayerView(Context context) {
        super(context);
    }

    public GPlayerView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    private void initGPlayer(String url, final boolean mediaCodec) {
        if (mGPlayer != null) {
            LogUtils.e("GPlayerView", "initGPlayer has been init");
            return;
        }
        mGPlayer = new GPlayer(this, url, mediaCodec);
        mGPlayer.setPlayStateChangedListener(this);
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
    public void setPlayStateChangedListener(OnStateChangedListener listener) {
        mPlayStateChangedListener = listener;
    }

    /**
     * get the media info
     *
     * @return media info
     */
    public MediaInfo getMediaInfo() {
        return mGPlayer.getMediaInfo();
    }

    /**
     * get player url
     *
     * @return url
     */
    public String getUrl() {
        return mGPlayer.getUrl();
    }

    /**
     * set the file url(only support local file now)
     *
     * @param url  file path
     */
    public void setUrl(String url) {
        setUrl(url, false);
    }

    /**
     * set the file url(only support local file now)
     *
     * @param url        file path
     * @param mediaCodec true : use mediacodec，false : use ffmpeg. valid when decode is true
     */
    public void setUrl(String url, boolean mediaCodec) {
        initGPlayer(url, mediaCodec);
    }

    @Override
    public void onStateChanged(GPlayer.State state) {
        if (mPlayStateChangedListener != null) {
            mPlayStateChangedListener.onStateChanged(state);
        }
        if (state == GPlayer.State.PLAYING) {
            post(new Runnable() {
                @Override
                public void run() {
                    resize();
                }
            });
        }
    }

    private void resize() {
        MediaInfo mediaInfo = mGPlayer.getMediaInfo();
        int width = mediaInfo.getInteger(MediaInfo.KEY_WIDTH, 16);
        int height = mediaInfo.getInteger(MediaInfo.KEY_HEIGHT, 9);
        int rotate = mediaInfo.getInteger(MediaInfo.KEY_VIDEO_ROTATE, 0);
        ViewGroup.LayoutParams params = getLayoutParams();
        params.width = getWidth();
        if (rotate == 90 || rotate == 270) {
            params.height = (int) (params.width * (width * 1.0f / height));
        } else {
            params.height = (int) (params.width * (height * 1.0f / width));
        }
        setLayoutParams(params);
        LogUtils.i(TAG, "resize to " + params.width + " " + params.height);
    }
}
