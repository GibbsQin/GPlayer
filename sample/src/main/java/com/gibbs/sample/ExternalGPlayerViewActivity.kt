package com.gibbs.sample

import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.view.WindowManager
import androidx.appcompat.app.AlertDialog
import com.gibbs.gplayer.GPlayer
import com.gibbs.gplayer.GPlayer.PlayStateChangedListener
import com.gibbs.gplayer.media.MediaInfo
import com.gibbs.gplayer.source.OnErrorListener
import com.gibbs.gplayer.source.OnSourceSizeChangedListener
import com.gibbs.gplayer.source.OnTimeChangedListener
import com.gibbs.gplayer.utils.LogUtils
import com.google.android.material.snackbar.Snackbar
import kotlinx.android.synthetic.main.activity_external_gplayer.*
import kotlinx.android.synthetic.main.activity_external_gplayer.gl_surface_view
import kotlinx.android.synthetic.main.layout_gplayer_top.*

class ExternalGPlayerViewActivity : BaseActivity(), PlayStateChangedListener,
        OnSourceSizeChangedListener, OnTimeChangedListener, OnErrorListener {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        setContentView(R.layout.activity_external_gplayer)
        val useMediaCodec = intent.getBooleanExtra("useMediaCodec", false)
        val url = intent.getStringExtra("url")
        val name = intent.getStringExtra("name")
        LogUtils.i(TAG, "url = $url, name = $name")
        LogUtils.i(TAG, "useMediaCodec = $useMediaCodec")
        gl_surface_view.url = url
        gl_surface_view.setPlayStateChangedListener(this)
        gl_surface_view.setOnErrorListener(this)
        gl_surface_view.setOnTimeChangedListener(this)
        gl_surface_view.setOnSourceSizeChangedListener(this)
        if (name != null) {
            title = name
        } else if (url != null) {
            val urlSplit = url.split("/".toRegex()).toTypedArray()
            title = urlSplit[urlSplit.size - 1]
        }
    }

    override fun onResume() {
        LogUtils.i(TAG, "onResume")
        super.onResume()
        gl_surface_view.onResume()
        gl_surface_view.startPlay()
    }

    override fun onPause() {
        LogUtils.i(TAG, "onPause")
        super.onPause()
        gl_surface_view.onPause()
        gl_surface_view.stopPlay()
    }

    override fun onPlayStateChanged(state: GPlayer.State) {
        LogUtils.i(TAG, "onPlayStateChanged $state")
        if (state == GPlayer.State.PLAYING) {
            runOnUiThread {
                val mediaInfo = gl_surface_view.mediaInfo
                val durationMs = mediaInfo.getInteger(MediaInfo.KEY_DURATION, 24)
                video_playback_seek_view.setDuration(durationMs)
            }
        } else if (state == GPlayer.State.IDLE) {
            finish()
        }
    }

    override fun onLocalAudioSizeChanged(size: Int) {
        runOnUiThread { audio_frame_progress.progress = size }
    }

    override fun onLocalVideoSizeChanged(size: Int) {
        runOnUiThread { video_frame_progress.progress = size }
    }

    override fun onRemoteAudioSizeChanged(size: Int) {
        runOnUiThread { audio_packet_progress.progress = size }
    }

    override fun onRemoteVideoSizeChanged(size: Int) {
        runOnUiThread { video_packet_progress.progress = size }
    }

    override fun onAudioTimeChanged(timeUs: Long) {}
    override fun onVideoTimeChanged(timeUs: Long) {
        runOnUiThread {
            val progress = (timeUs / 1000).toInt()
            if (progress > video_playback_seek_view.progress) {
                video_playback_seek_view.progress = progress
            }
        }
    }

    override fun onError(errorCode: Int, errorMessage: String) {
        LogUtils.i(TAG, "onError $errorCode $errorMessage")
        Snackbar.make(gl_surface_view, errorMessage, Snackbar.LENGTH_SHORT).show()
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.info_menu, menu)
        return super.onCreateOptionsMenu(menu)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        if (item.itemId == R.id.action_menu_info) {
            showInfoDialog()
            return true
        }
        return super.onOptionsItemSelected(item)
    }

    private fun showInfoDialog() {
        val mediaInfo = gl_surface_view.mediaInfo
        val width = mediaInfo.getInteger(MediaInfo.KEY_WIDTH, 16)
        val height = mediaInfo.getInteger(MediaInfo.KEY_HEIGHT, 9)
        val sampleRate = mediaInfo.getInteger(MediaInfo.KEY_AUDIO_SAMPLE_RATE, 8000)
        val numPerSample = mediaInfo.getInteger(MediaInfo.KEY_AUDIO_SAMPLE_NUM_PERFRAME, 1024)
        val frameRate = mediaInfo.getInteger(MediaInfo.KEY_FRAME_RATE, 24)
        val audioRate = if (numPerSample > 0) sampleRate / numPerSample else 0
        val msg = String.format("audio rate:%s, video rate:%s, %s:%s\n%s", audioRate, frameRate,
                width, height, mediaInfo.toString())
        AlertDialog.Builder(this)
                .setTitle("Media info")
                .setMessage(msg)
                .show()
    }

    companion object {
        private const val TAG = "ExternalGPlayerViewActivity"
    }
}