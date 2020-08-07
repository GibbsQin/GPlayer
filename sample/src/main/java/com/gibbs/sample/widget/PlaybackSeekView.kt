package com.gibbs.sample.widget

import android.content.Context
import android.util.AttributeSet
import android.view.LayoutInflater
import android.widget.LinearLayout
import android.widget.SeekBar
import android.widget.SeekBar.OnSeekBarChangeListener
import com.gibbs.sample.R
import kotlinx.android.synthetic.main.layout_playback_seek.view.*

class PlaybackSeekView : LinearLayout {
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
        LayoutInflater.from(context).inflate(R.layout.layout_playback_seek, this)
        sb_playback_seek.setOnSeekBarChangeListener(object : OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
                tv_current_progress.text = second2Text(progress)
            }

            override fun onStartTrackingTouch(seekBar: SeekBar) {}
            override fun onStopTrackingTouch(seekBar: SeekBar) {}
        })
    }

    fun setDuration(durationMs: Int) {
        sb_playback_seek.progress = 0
        sb_playback_seek.max = durationMs
        tv_duration.text = second2Text(durationMs)
    }

    var progress: Int
        get() = sb_playback_seek.progress
        set(progressMs) {
            sb_playback_seek.progress = progressMs
        }

    private fun second2Text(secondMs: Int): String {
        val second = secondMs / 1000
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