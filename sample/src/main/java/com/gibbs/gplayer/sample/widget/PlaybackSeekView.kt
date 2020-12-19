package com.gibbs.gplayer.sample.widget

import android.content.Context
import android.util.AttributeSet
import android.view.LayoutInflater
import android.widget.LinearLayout
import android.widget.SeekBar
import android.widget.SeekBar.OnSeekBarChangeListener
import com.gibbs.gplayer.utils.LogUtils
import com.gibbs.gplayer.sample.R
import kotlinx.android.synthetic.main.layout_playback_seek.view.*

class PlaybackSeekView : LinearLayout {
    private var mOnSeekChangeListener: OnSeekChangeListener? = null

    var progress: Int
        get() = sb_playback_seek.progress
        set(progressUs) {
            LogUtils.i("PlaybackSeekView", "setProgress $progressUs")
            sb_playback_seek.progress = progressUs
        }

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
            override fun onStopTrackingTouch(seekBar: SeekBar) {
                mOnSeekChangeListener?.onProgressChanged(progress)
            }
        })
    }

    fun setDuration(durationUs: Int) {
        LogUtils.i("PlaybackSeekView", "setDuration $durationUs")
        sb_playback_seek.progress = 0
        sb_playback_seek.max = durationUs
        tv_duration.text = second2Text(durationUs)
    }

    private fun second2Text(secondUs: Int): String {
        val second = secondUs / 1000_000
        val m = second / 60
        val s = second % 60
        return String.format("%s:%s", formatTime(m), formatTime(s))
    }

    private fun formatTime(time: Int): String = if (time < 10) "0$time" else time.toString()

    public fun setOnSeekChangeListener(listener: OnSeekChangeListener) {
        mOnSeekChangeListener = listener;
    }

    public interface OnSeekChangeListener {
        fun onProgressChanged(progressUs: Int)
    }
}