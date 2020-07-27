package com.gibbs.sample;

import android.content.Intent;
import android.opengl.GLSurfaceView;
import android.os.Bundle;

import com.gibbs.gplayer.GPlayer;
import com.gibbs.gplayer.source.MediaSource;
import com.gibbs.gplayer.utils.LogUtils;

public class GPlayerActivity extends BaseActivity {
    private static final String TAG = "GPlayerActivity";

    private GLSurfaceView mVideoView;
    private GPlayer mGPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_gl_surface);
        mVideoView = findViewById(R.id.gl_surface_view);
        Intent intent = getIntent();
        boolean decodeSource = intent.getBooleanExtra("decodeSource", false);
        boolean useMediaCodec = intent.getBooleanExtra("useMediaCodec", false);
        String url = intent.getStringExtra("url");
        LogUtils.i(TAG, "url = " + url);
        LogUtils.i(TAG, "decodeSource = " + decodeSource + ", useMediaCodec = " + useMediaCodec);
        mGPlayer = new GPlayer(mVideoView, MediaSource.SOURCE_TYPE_FILE, url, decodeSource, useMediaCodec);
    }

    @Override
    protected void onResume() {
        LogUtils.i(TAG, "onResume");
        super.onResume();
        if (mVideoView != null) {
            mVideoView.onResume();
            mGPlayer.prepare();
        }
    }

    @Override
    protected void onPause() {
        LogUtils.i(TAG, "onPause");
        super.onPause();
        if (mVideoView != null) {
            mVideoView.onPause();
            mGPlayer.finish();
        }
    }
}