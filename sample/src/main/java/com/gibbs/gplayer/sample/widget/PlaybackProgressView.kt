package com.gibbs.gplayer.sample.widget

import android.content.Context
import android.util.AttributeSet
import android.view.LayoutInflater
import android.widget.LinearLayout
import android.widget.ProgressBar
import android.widget.TextView
import com.gibbs.gplayer.sample.R

class PlaybackProgressView : LinearLayout {
    private var mPBDuration: ProgressBar? = null
    private var mTvCurrentProgress: TextView? = null
    private var mTvDuration: TextView? = null
    private var reverse = false
    private var showAsTime = false
    private var title: String? = null

    constructor(context: Context) : super(context) {
        initView(context, null)
    }

    constructor(context: Context, attrs: AttributeSet?) : super(context, attrs) {
        initView(context, attrs)
    }

    constructor(context: Context, attrs: AttributeSet?, defStyleAttr: Int) : super(context, attrs, defStyleAttr) {
        initView(context, attrs)
    }

    constructor(context: Context, attrs: AttributeSet?, defStyleAttr: Int, defStyleRes: Int) : super(context, attrs, defStyleAttr, defStyleRes) {
        initView(context, attrs)
    }

    private fun initView(context: Context, attrs: AttributeSet?) {
        if (attrs != null) {
            val attrTypedArray = context.obtainStyledAttributes(attrs, R.styleable.PlaybackProgressView)
            reverse = attrTypedArray.getBoolean(R.styleable.PlaybackProgressView_reverse, false)
            showAsTime = attrTypedArray.getBoolean(R.styleable.PlaybackProgressView_showAsTime, false)
            title = attrTypedArray.getString(R.styleable.PlaybackProgressView_title)
            attrTypedArray.recycle()
        }
        LayoutInflater.from(context).inflate(R.layout.layout_playback_progress, this)
        mPBDuration = findViewById(R.id.pb_playback_progress)
        if (reverse) {
            mTvCurrentProgress = findViewById(R.id.tv_duration)
            mTvDuration = findViewById(R.id.tv_current_progress)
        } else {
            mTvCurrentProgress = findViewById(R.id.tv_current_progress)
            mTvDuration = findViewById(R.id.tv_duration)
        }
        mTvDuration?.text = title
    }

    fun setDuration(duration: Int) {
        mTvCurrentProgress!!.text = if (showAsTime) second2Text(0) else "0"
        mPBDuration!!.progress = 0
        mTvDuration!!.text = if (showAsTime) second2Text(duration) else duration.toString()
        mPBDuration!!.max = duration
    }

    var progress: Int
        get() = mPBDuration!!.progress
        set(progress) {
            mTvCurrentProgress!!.text = if (showAsTime) second2Text(progress) else progress.toString()
            mPBDuration!!.progress = progress
        }

    private fun second2Text(second: Int): String {
        val m = second / 60
        val s = second % 60
        return String.format("%s:%s", formatTime(m), formatTime(s))
    }

    private fun formatTime(time: Int): String {
        return if (time < 10) {
            "0$time"
        } else {
            time.toString()
        }
    }
}