package com.gibbs.gplayer.sample.widget;

import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;

import com.gibbs.gplayer.GPlayer;
import com.gibbs.gplayer.GPlayerView;
import com.gibbs.gplayer.R;

public class VideoView extends ConstraintLayout implements GPlayer.OnStateChangedListener, GPlayer.OnPositionChangedListener {
    private GPlayerView playerView;
    private SeekBar seekBar;
    private ProgressBar progressBar;
    private TextView progressView;
    private TextView durationView;

    private GPlayer.OnStateChangedListener mOnStateChangedListener;
    private GPlayer.OnPositionChangedListener mOnPositionChangedListener;

    public VideoView(Context context) {
        this(context, null);
    }

    public VideoView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public VideoView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        View view = LayoutInflater.from(context).inflate(R.layout.view_video, this, true);
        playerView = view.findViewById(R.id.player_view);
        seekBar = view.findViewById(R.id.sb_playback_seek);
        progressBar = view.findViewById(R.id.progress_bar);
        progressView = view.findViewById(R.id.tv_current_progress);
        durationView = view.findViewById(R.id.tv_duration);
        playerView.setOnStateChangedListener(this);
        playerView.setOnPositionChangedListener(this);
    }

    public GPlayerView getPlayerView() {
        return playerView;
    }

    public void setDataSource(String url) {
        playerView.setDataSource(url);
    }

    public void prepare() {
        playerView.prepare();
    }

    public void start() {
        playerView.start();
    }

    public void stop() {
        playerView.stop();
    }

    public void pause() {
        playerView.pause();
    }

    public void release() {
        playerView.release();
    }

    public void seekTo(int secondMs) {
        playerView.seekTo(secondMs);
    }

    public boolean isPlaying() {
        return playerView.isPlaying();
    }

    public void setFlags(int flags) {
        playerView.setFlags(flags);
    }

    public int getCurrentPosition() {
        return playerView.getCurrentPosition();
    }

    public GPlayer.State getState() {
        return playerView.getState();
    }

    public void setOnPreparedListener(GPlayer.OnPreparedListener listener) {
        playerView.setOnPreparedListener(listener);
    }

    public void setOnErrorListener(GPlayer.OnErrorListener listener) {
        playerView.setOnErrorListener(listener);
    }

    public void setOnStateChangedListener(GPlayer.OnStateChangedListener listener) {
        mOnStateChangedListener = listener;
    }

    public void setOnPositionChangedListener(GPlayer.OnPositionChangedListener listener) {
        mOnPositionChangedListener = listener;
    }

    public void setOnBufferChangedListener(GPlayer.OnBufferChangedListener listener) {
        playerView.setOnBufferChangedListener(listener);
    }

    public void onPositionChanged(int timeMs) {
        if (mOnPositionChangedListener != null) {
            mOnPositionChangedListener.onPositionChanged(timeMs);
        }
        seekBar.setProgress(timeMs);
        progressView.setText(second2Text(timeMs));
    }

    public void onStateChanged(GPlayer.State state) {
        if (mOnStateChangedListener != null) {
            mOnStateChangedListener.onStateChanged(state);
        }
        if (state == GPlayer.State.PREPARING || state == GPlayer.State.PAUSED || state == GPlayer.State.STOPPING) {
            progressBar.setVisibility(VISIBLE);
        } else if (state == GPlayer.State.PREPARED) {
            progressBar.setVisibility(VISIBLE);
        } else {
            progressBar.setVisibility(GONE);
        }
    }

    private String second2Text(int secondUs) {
        secondUs = Math.abs(secondUs) / 1000_000;
        int m = (secondUs / 60) % 60;
        int s = secondUs % 60;
        return String.format("%s:%s", formatTime(m), formatTime(s));
    }

    private String formatTime(int time) {
        return (time < 10) ? "0" + time : String.valueOf(time);
    }
}
