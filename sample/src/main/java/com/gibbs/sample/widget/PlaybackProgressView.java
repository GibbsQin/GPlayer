package com.gibbs.sample.widget;

import android.content.Context;
import android.content.res.TypedArray;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

import androidx.annotation.Nullable;

import com.gibbs.sample.R;

public class PlaybackProgressView extends LinearLayout {
    private ProgressBar mPBDuration;
    private TextView mTvCurrentProgress;
    private TextView mTvDuration;

    private boolean reverse;
    private boolean showAsTime;
    private String title;

    public PlaybackProgressView(Context context) {
        super(context);
        initView(context, null);
    }

    public PlaybackProgressView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        initView(context, attrs);
    }

    public PlaybackProgressView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initView(context, attrs);
    }

    public PlaybackProgressView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        initView(context, attrs);
    }

    private void initView(Context context, AttributeSet attrs) {
        if (attrs != null) {
            TypedArray attrTypedArray = context.obtainStyledAttributes(attrs, R.styleable.PlaybackProgressView);
            reverse = attrTypedArray.getBoolean(R.styleable.PlaybackProgressView_reverse, false);
            showAsTime = attrTypedArray.getBoolean(R.styleable.PlaybackProgressView_showAsTime, false);
            title = attrTypedArray.getString(R.styleable.PlaybackProgressView_title);
            attrTypedArray.recycle();
        }

        LayoutInflater.from(context).inflate(R.layout.layout_playback_progress, this);
        mPBDuration = findViewById(R.id.pb_playback_progress);
        if (reverse) {
            mTvCurrentProgress = findViewById(R.id.tv_duration);
            mTvDuration = findViewById(R.id.tv_current_progress);
        } else {
            mTvCurrentProgress = findViewById(R.id.tv_current_progress);
            mTvDuration = findViewById(R.id.tv_duration);
        }
        mTvDuration.setText(title);
    }

    public void setDuration(int duration) {
        mTvCurrentProgress.setText(showAsTime ? second2Text(0) : "0");
        mPBDuration.setProgress(0);
        mTvDuration.setText(showAsTime ? second2Text(duration) : String.valueOf(duration));
        mPBDuration.setMax(duration);
    }

    public void setProgress(int progress) {
        mTvCurrentProgress.setText(showAsTime ? second2Text(progress) : String.valueOf(progress));
        mPBDuration.setProgress(progress);
    }

    public int getProgress() {
        return mPBDuration.getProgress();
    }

    private String second2Text(int second) {
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
