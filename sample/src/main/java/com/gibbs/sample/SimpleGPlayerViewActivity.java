package com.gibbs.sample;

import android.content.Intent;
import android.os.Bundle;

import com.gibbs.gplayer.GPlayerView;
import com.gibbs.gplayer.source.MediaSource;

public class SimpleGPlayerViewActivity extends BaseActivity {
    private GPlayerView mVideoView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_simple_g_player);
        mVideoView = findViewById(R.id.gl_surface_view);
        Intent intent = getIntent();
        String url = intent.getStringExtra("url");
        mVideoView.setUrl(MediaSource.SOURCE_TYPE_FILE, url);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mVideoView != null) {
            mVideoView.onResume();
            mVideoView.startPlay();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mVideoView != null) {
            mVideoView.onPause();
            mVideoView.stopPlay();
        }
    }
}