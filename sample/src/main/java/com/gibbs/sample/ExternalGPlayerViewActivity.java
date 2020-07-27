package com.gibbs.sample;

import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.WindowManager;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.constraintlayout.widget.ConstraintLayout;

import com.gibbs.gplayer.GPlayer;
import com.gibbs.gplayer.GPlayerView;
import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.source.MediaSource;
import com.gibbs.gplayer.source.OnErrorListener;
import com.gibbs.gplayer.source.OnSourceSizeChangedListener;
import com.gibbs.gplayer.source.OnTimeChangedListener;
import com.gibbs.gplayer.utils.LogUtils;
import com.gibbs.sample.widget.PlaybackProgressView;
import com.gibbs.sample.widget.PlaybackSeekView;
import com.google.android.material.snackbar.Snackbar;

public class ExternalGPlayerViewActivity extends BaseActivity implements GPlayer.PlayStateChangedListener,
        OnSourceSizeChangedListener, OnTimeChangedListener, OnErrorListener {
    private static final String TAG = "ExternalGPlayerViewActivity";

    private GPlayerView mVideoView;
    private PlaybackProgressView mAudioPacketProgressView;
    private PlaybackProgressView mVideoPacketProgressView;
    private PlaybackProgressView mAudioFrameProgressView;
    private PlaybackProgressView mVideoFrameProgressView;
    private PlaybackSeekView mVideoDurationSeekView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_gplayer);
        mVideoView = findViewById(R.id.gl_surface_view);
        mAudioPacketProgressView = findViewById(R.id.audio_packet_progress);
        mVideoPacketProgressView = findViewById(R.id.video_packet_progress);
        mAudioFrameProgressView = findViewById(R.id.audio_frame_progress);
        mVideoFrameProgressView = findViewById(R.id.video_frame_progress);
        mVideoDurationSeekView = findViewById(R.id.video_playback_seek_view);
        Intent intent = getIntent();
        boolean decodeSource = intent.getBooleanExtra("decodeSource", false);
        boolean useMediaCodec = intent.getBooleanExtra("useMediaCodec", false);
        String url = intent.getStringExtra("url");
        LogUtils.i(TAG, "url = " + url);
        LogUtils.i(TAG, "decodeSource = " + decodeSource + ", useMediaCodec = " + useMediaCodec);

        mVideoView.setUrl(MediaSource.SOURCE_TYPE_FILE, url, decodeSource, useMediaCodec);
        mVideoView.setPlayStateChangedListener(this);
        mVideoView.setOnErrorListener(this);
        mVideoView.setOnTimeChangedListener(this);
        mVideoView.setOnSourceSizeChangedListener(this);
        if (url != null) {
            String[] urlSplit = url.split("/");
            setTitle(urlSplit[urlSplit.length - 1]);
        }
    }

    @Override
    protected void onResume() {
        LogUtils.i(TAG, "onResume");
        super.onResume();
        if (mVideoView != null) {
            mVideoView.onResume();
            mVideoView.startPlay();
        }
    }

    @Override
    protected void onPause() {
        LogUtils.i(TAG, "onPause");
        super.onPause();
        if (mVideoView != null) {
            mVideoView.onPause();
            mVideoView.stopPlay();
        }
    }

    @Override
    public void onPlayStateChanged(GPlayer.State state) {
        LogUtils.i(TAG, "onPlayStateChanged " + state);
        if (state == GPlayer.State.PLAYING) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    MediaInfo mediaInfo = mVideoView.getMediaInfo();
                    int width = mediaInfo.getInteger(MediaInfo.KEY_WIDTH, 16);
                    int height = mediaInfo.getInteger(MediaInfo.KEY_HEIGHT, 9);
                    ConstraintLayout.LayoutParams params = (ConstraintLayout.LayoutParams) mVideoView.getLayoutParams();
                    params.dimensionRatio = String.format("H, %s:%s", width, height);

                    int durationMs = mediaInfo.getInteger(MediaInfo.KEY_DURATION, 24);
                    mVideoDurationSeekView.setDuration(durationMs);
                }
            });
        } else if (state == GPlayer.State.IDLE) {
            finish();
        }
    }

    @Override
    public void onLocalAudioSizeChanged(final int size) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mAudioFrameProgressView.setProgress(size);
            }
        });
    }

    @Override
    public void onLocalVideoSizeChanged(final int size) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mVideoFrameProgressView.setProgress(size);
            }
        });
    }

    @Override
    public void onRemoteAudioSizeChanged(final int size) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mAudioPacketProgressView.setProgress(size);
            }
        });
    }

    @Override
    public void onRemoteVideoSizeChanged(final int size) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mVideoPacketProgressView.setProgress(size);
            }
        });
    }

    @Override
    public void onAudioTimeChanged(final long timeUs) {

    }

    @Override
    public void onVideoTimeChanged(final long timeUs) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                int progress = (int) (timeUs / 1_000);
                if (progress > mVideoDurationSeekView.getProgress()) {
                    mVideoDurationSeekView.setProgress(progress);
                }
            }
        });
    }

    @Override
    public void onError(int errorCode, String errorMessage) {
        LogUtils.i(TAG, "onError " + errorCode + " " + errorMessage);
        Snackbar.make(mVideoView, errorMessage, Snackbar.LENGTH_SHORT).show();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.info_menu, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        if (item.getItemId() == R.id.action_menu_info) {
            showInfoDialog();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private void showInfoDialog() {
        MediaInfo mediaInfo = mVideoView.getMediaInfo();
        int width = mediaInfo.getInteger(MediaInfo.KEY_WIDTH, 16);
        int height = mediaInfo.getInteger(MediaInfo.KEY_HEIGHT, 9);
        int sampleRate = mediaInfo.getInteger(MediaInfo.KEY_AUDIO_SAMPLE_RATE, 8000);
        int numPerSample = mediaInfo.getInteger(MediaInfo.KEY_AUDIO_SAMPLE_NUM_PERFRAME, 1024);
        int frameRate = mediaInfo.getInteger(MediaInfo.KEY_FRAME_RATE, 24);
        int audioRate = numPerSample > 0 ? (sampleRate / numPerSample) : 0;
        String msg = String.format("audio rate:%s, video rate:%s, %s:%s\n%s", audioRate, frameRate,
                width, height, mediaInfo.toString());
        new AlertDialog.Builder(this)
                .setTitle("Media info")
                .setMessage(msg)
                .show();
    }
}
