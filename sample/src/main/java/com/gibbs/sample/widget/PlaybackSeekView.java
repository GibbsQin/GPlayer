package com.gibbs.sample.widget;

import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.annotation.Nullable;

import com.gibbs.sample.R;

public class PlaybackSeekView extends LinearLayout {
    private SeekBar mSeekBarDuration;
    private TextView mTvCurrentProgress;
    private TextView mTvDuration;

    public PlaybackSeekView(Context context) {
        super(context);
        initView(context, null);
    }

    public PlaybackSeekView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        initView(context, attrs);
    }

    public PlaybackSeekView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initView(context, attrs);
    }

    public PlaybackSeekView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        initView(context, attrs);
    }

    private void initView(Context context, AttributeSet attrs) {
        LayoutInflater.from(context).inflate(R.layout.layout_playback_seek, this);
        mSeekBarDuration = findViewById(R.id.sb_playback_seek);
        mSeekBarDuration.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mTvCurrentProgress.setText(second2Text(progress));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
        mTvCurrentProgress = findViewById(R.id.tv_current_progress);
        mTvDuration = findViewById(R.id.tv_duration);
    }

    public void setDuration(int durationMs) {
        mSeekBarDuration.setProgress(0);
        mSeekBarDuration.setMax(durationMs);
        mTvDuration.setText(second2Text(durationMs));
    }

    public void setProgress(int progressMs) {
        mSeekBarDuration.setProgress(progressMs);
    }

    public int getProgress() {
        return mSeekBarDuration.getProgress();
    }

    private String second2Text(int secondMs) {
        int second = secondMs / 1000;
        int m = second / 60;
        int s = second % 60;
        return String.format("%s:%s", formatTime(m), formatTime(s));
    }

    private String formatTime(int time) {
        if (time < 10) {
            return "0" + time;
        } else {
            return String.valueOf(time);
        }
    }
}

