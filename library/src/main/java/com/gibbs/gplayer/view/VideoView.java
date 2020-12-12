package com.gibbs.gplayer.view;

import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.SurfaceView;
import android.view.View;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;

import com.gibbs.gplayer.GPlayer;
import com.gibbs.gplayer.GPlayerView;
import com.gibbs.gplayer.IGPlayer;
import com.gibbs.gplayer.R;
import com.gibbs.gplayer.listener.OnBufferChangedListener;
import com.gibbs.gplayer.listener.OnErrorListener;
import com.gibbs.gplayer.listener.OnPositionChangedListener;
import com.gibbs.gplayer.listener.OnPreparedListener;
import com.gibbs.gplayer.listener.OnStateChangedListener;

public class VideoView extends ConstraintLayout implements IGPlayer, OnStateChangedListener, OnPositionChangedListener {
    private GPlayerView playerView;
    private SeekBar seekBar;
    private ProgressBar progressBar;
    private TextView progressView;
    private TextView durationView;

    private OnStateChangedListener mOnStateChangedListener;
    private OnPositionChangedListener mOnPositionChangedListener;

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

    @Override
    public void setSurface(SurfaceView view) {

    }

    @Override
    public void setDataSource(String url) {
        playerView.setDataSource(url);
    }

    @Override
    public void prepare() {
        playerView.prepare();
    }

    @Override
    public void start() {
        playerView.start();
    }

    @Override
    public void stop() {
        playerView.stop();
    }

    @Override
    public void pause() {
        playerView.pause();
    }

    @Override
    public void release() {
        playerView.release();
    }

    @Override
    public void seekTo(int secondMs) {
        playerView.seekTo(secondMs);
    }

    @Override
    public boolean isPlaying() {
        return playerView.isPlaying();
    }

    @Override
    public void setFlags(int flags) {
        playerView.setFlags(flags);
    }

    @Override
    public int getCurrentPosition() {
        return playerView.getCurrentPosition();
    }

    @Override
    public int getDuration() {
        return playerView.getDuration();
    }

    @Override
    public int getVideoWidth() {
        return playerView.getVideoWidth();
    }

    @Override
    public int getVideoHeight() {
        return playerView.getVideoHeight();
    }

    @Override
    public int getVideoRotate() {
        return playerView.getVideoRotate();
    }

    @Override
    public GPlayer.State getState() {
        return playerView.getState();
    }

    @Override
    public void setOnPreparedListener(OnPreparedListener listener) {
        playerView.setOnPreparedListener(listener);
    }

    @Override
    public void setOnErrorListener(OnErrorListener listener) {
        playerView.setOnErrorListener(listener);
    }

    @Override
    public void setOnStateChangedListener(OnStateChangedListener listener) {
        mOnStateChangedListener = listener;
    }

    @Override
    public void setOnPositionChangedListener(OnPositionChangedListener listener) {
        mOnPositionChangedListener = listener;
    }

    @Override
    public void setOnBufferChangedListener(OnBufferChangedListener listener) {
        playerView.setOnBufferChangedListener(listener);
    }

    @Override
    public void onPositionChanged(int timeMs) {
        if (mOnPositionChangedListener != null) {
            mOnPositionChangedListener.onPositionChanged(timeMs);
        }
        seekBar.setProgress(timeMs);
        progressView.setText(second2Text(timeMs));
    }

    @Override
    public void onStateChanged(GPlayer.State state) {
        if (mOnStateChangedListener != null) {
            mOnStateChangedListener.onStateChanged(state);
        }
        if (state == GPlayer.State.PREPARING || state == GPlayer.State.PAUSED || state == GPlayer.State.STOPPING) {
            progressBar.setVisibility(VISIBLE);
        } else if (state == GPlayer.State.PREPARED) {
            progressBar.setVisibility(VISIBLE);
            int duration = getDuration();
            seekBar.setMax(duration);
            durationView.setText(second2Text(duration));
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
