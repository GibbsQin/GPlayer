package com.gibbs.sample

import android.os.Bundle
import android.view.WindowManager
import com.gibbs.gplayer.GPlayer
import com.gibbs.gplayer.listener.*
import com.gibbs.gplayer.utils.LogUtils
import kotlinx.android.synthetic.main.activity_external_gplayer.*
import kotlinx.android.synthetic.main.layout_gplayer_top.*

class BufferTestActivity : BaseActivity(), OnPreparedListener, OnStateChangedListener,
        OnBufferChangedListener, OnPositionChangedListener, OnErrorListener {
    private val mGPlayer = GPlayer()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        setContentView(R.layout.activity_buffer_test)
        val useMediaCodec = intent.getBooleanExtra("useMediaCodec", false)
        val url = intent.getStringExtra("url")
        val name = intent.getStringExtra("name")
        LogUtils.i(TAG, "url = $url, name = $name")
        LogUtils.i(TAG, "useMediaCodec = $useMediaCodec")
        mGPlayer.setDataSource(url)
        mGPlayer.setOnPreparedListener(this)
        mGPlayer.setOnStateChangedListener(this)
        mGPlayer.setOnErrorListener(this)
        mGPlayer.setOnPositionChangedListener(this)
        mGPlayer.setOnBufferChangedListener(this)
    }

    override fun onStart() {
        super.onStart()
        LogUtils.i(TAG, "onStart")
        mGPlayer.prepare()
    }

    override fun onStop() {
        super.onStop()
        LogUtils.i(TAG, "onStop")
        mGPlayer.stop()
    }

    override fun onPrepared() {
        mGPlayer.start()
    }

    override fun onStateChanged(state: GPlayer.State) {
        LogUtils.i(TAG, "onPlayStateChanged $state")
        if (state == GPlayer.State.PLAYING) {
            runOnUiThread {
                LogUtils.i(TAG, "onPlayStateChanged duration = ${mGPlayer.duration}")
                video_playback_seek_view.setDuration(mGPlayer.duration)
            }
        } else if (state == GPlayer.State.IDLE) {
            finish()
        }
    }

    override fun onAudioFrameSizeChanged(size: Int) {
        runOnUiThread { audio_frame_progress.progress = size }
    }

    override fun onVideoFrameSizeChanged(size: Int) {
        runOnUiThread { video_frame_progress.progress = size }
    }

    override fun onAudioPacketSizeChanged(size: Int) {
        runOnUiThread { audio_packet_progress.progress = size }
    }

    override fun onVideoPacketSizeChanged(size: Int) {
        runOnUiThread { video_packet_progress.progress = size }
    }

    override fun onPositionChanged(progress: Int) {
        LogUtils.i(TAG, "onPositionChanged progress = $progress")
        runOnUiThread {
            if (progress > video_playback_seek_view.progress) {
                video_playback_seek_view.progress = progress
            }
        }
    }

    override fun onError(errorCode: Int, errorMessage: String) {
        LogUtils.i(TAG, "onError $errorCode $errorMessage")
    }

    companion object {
        private const val TAG = "BufferTestActivity"
    }
}