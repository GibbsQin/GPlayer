package com.gibbs.gplayer.sample.widget;

import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;

import com.gibbs.gplayer.GPlayer;
import com.gibbs.gplayer.GPlayerView;
import com.gibbs.gplayer.sample.R;

public class GVideoView extends ConstraintLayout implements View.OnClickListener, SeekBar.OnSeekBarChangeListener,
        GPlayer.OnStateChangedListener, GPlayer.OnPositionChangedListener, GPlayer.OnBufferChangedListener,
        GPlayer.OnErrorListener, GPlayer.OnCompletedListener {
    private final GPlayerView playerView;
    private final SeekBar seekBar;
    private final ProgressBar progressBar;
    private final TextView progressView;
    private final TextView durationView;
    private final TextView stateView;
    private final TextView errorView;
    private final ImageView playBtn;
    private final ImageView pauseBtn;

    private GPlayer.OnStateChangedListener mOnStateChangedListener;
    private GPlayer.OnPositionChangedListener mOnPositionChangedListener;

    public GVideoView(Context context) {
        this(context, null);
    }

    public GVideoView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public GVideoView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        View view = LayoutInflater.from(context).inflate(R.layout.view_video, this, true);
        playerView = view.findViewById(R.id.player_view);
        seekBar = view.findViewById(R.id.sb_playback_seek);
        progressBar = view.findViewById(R.id.progress_bar);
        progressView = view.findViewById(R.id.tv_current_progress);
        durationView = view.findViewById(R.id.tv_duration);
        stateView = view.findViewById(R.id.tv_state);
        errorView = view.findViewById(R.id.tv_error);
        pauseBtn = view.findViewById(R.id.iv_pause);
        playBtn = view.findViewById(R.id.iv_play);
        pauseBtn.setOnClickListener(this);
        playBtn.setOnClickListener(this);
        seekBar.setOnSeekBarChangeListener(this);
        playerView.setOnStateChangedListener(this);
        playerView.setOnPositionChangedListener(this);
        playerView.setOnBufferChangedListener(this);
        playerView.setOnErrorListener(this);
        playerView.setOnCompletedListener(this);
    }

    public GPlayerView getPlayerView() {
        return playerView;
    }

    public void setDataSource(String url) {
        playerView.setDataSource(url);
    }

    public void prepareAsync() {
        playerView.prepareAsync();
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

    public void reset() {
        playerView.reset();
    }

    public void seekTo(int secondMs) {
        playerView.seekTo(secondMs);
    }

    public void setLooping(boolean looping) {
        playerView.setLooping(looping);
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

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.iv_play) {
            if (getState() == GPlayer.State.PAUSED) {
                start();
            } else if (getState() == GPlayer.State.IDLE || getState() == GPlayer.State.STOPPED) {
                prepare();
            }
        } else if (v.getId() == R.id.iv_pause) {
            pause();
        }
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        playerView.seekTo(seekBar.getProgress());
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
        playBtn.setVisibility(state == GPlayer.State.STARTED ? GONE : VISIBLE);
        pauseBtn.setVisibility(state != GPlayer.State.STARTED ? GONE : VISIBLE);
        if (mOnStateChangedListener != null) {
            mOnStateChangedListener.onStateChanged(state);
        }
        if (state == GPlayer.State.PREPARING || state == GPlayer.State.PAUSED) {
            progressBar.setVisibility(VISIBLE);
        } else if (state == GPlayer.State.PREPARED) {
            progressBar.setVisibility(VISIBLE);
            durationView.setText(second2Text(playerView.getDuration()));
            seekBar.setMax(playerView.getDuration());
            errorView.setText(null);
            resize(playerView.getVideoWidth(), playerView.getVideoHeight());
        } else {
            progressBar.setVisibility(GONE);
        }
        stateView.setText(state.name());
    }

    @Override
    public void onError(int errorCode, String errorMessage) {
        errorView.setText(errorMessage);
    }

    @Override
    public void onBufferChanged(int percent) {
        if (percent == 0) {
            progressBar.setVisibility(VISIBLE);
        } else if (percent == 100 && progressBar.getVisibility() == VISIBLE) {
            progressBar.setVisibility(GONE);
        }
    }

    @Override
    public void onCompleted() {
        errorView.setText("EOF");
    }

    private String second2Text(int secondUs) {
        secondUs = Math.abs(secondUs);
        int m = (secondUs / 60) % 60;
        int s = secondUs % 60;
        return String.format("%s:%s", formatTime(m), formatTime(s));
    }

    private String formatTime(int time) {
        return (time < 10) ? "0" + time : String.valueOf(time);
    }

    private void resize(int width, int height) {
        LayoutParams params = (LayoutParams) playerView.getLayoutParams();
        params.dimensionRatio = String.format("H,%s:%s", width, height);
    }
}
