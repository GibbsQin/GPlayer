package com.gibbs.gplayer.sample

import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.view.WindowManager
import com.gibbs.gplayer.GPlayer
import com.gibbs.gplayer.listener.*
import com.gibbs.gplayer.utils.LogUtils
import com.gibbs.gplayer.sample.widget.PlaybackSeekView.OnSeekChangeListener
import com.google.android.material.snackbar.Snackbar
import kotlinx.android.synthetic.main.activity_external_gplayer.*
import kotlinx.android.synthetic.main.layout_gplayer_top.*

class ExternalGPlayerViewActivity : BaseActivity(), OnPreparedListener, OnStateChangedListener,
        OnBufferChangedListener, OnPositionChangedListener, OnErrorListener, OnSeekChangeListener {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        setContentView(R.layout.activity_external_gplayer)
        val useMediaCodec = intent.getBooleanExtra("useMediaCodec", false)
        val url = intent.getStringExtra("url")
        val name = intent.getStringExtra("name")
        LogUtils.i(TAG, "url = $url, name = $name")
        LogUtils.i(TAG, "useMediaCodec = $useMediaCodec")
        if (useMediaCodec) {
            gl_surface_view.setFlags(GPlayer.USE_MEDIA_CODEC)
        }
        gl_surface_view.setDataSource(url)
        gl_surface_view.setOnPreparedListener(this)
        gl_surface_view.setOnStateChangedListener(this)
        gl_surface_view.setOnErrorListener(this)
        gl_surface_view.setOnPositionChangedListener(this)
        gl_surface_view.setOnBufferChangedListener(this)
        video_playback_seek_view.setOnSeekChangeListener(this)
        if (name != null) {
            title = name
        } else if (url != null) {
            val urlSplit = url.split("/".toRegex()).toTypedArray()
            title = urlSplit[urlSplit.size - 1]
        }
    }

    override fun onResume() {
        super.onResume()
        LogUtils.i(TAG, "onResume")
        gl_surface_view.onResume()
        gl_surface_view.prepare()
    }

    override fun onPause() {
        super.onPause()
        LogUtils.i(TAG, "onPause")
        gl_surface_view.onPause()
        gl_surface_view.stop()
    }

    override fun onPrepared() {
        gl_surface_view.start()
    }

    override fun onStateChanged(state: GPlayer.State) {
        LogUtils.i(TAG, "onPlayStateChanged $state")
        if (state == GPlayer.State.PLAYING) {
            runOnUiThread {
                video_playback_seek_view.setDuration(gl_surface_view.duration)
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
        runOnUiThread {
            if (progress > video_playback_seek_view.progress) {
                video_playback_seek_view.progress = progress
            }
        }
    }

    override fun onError(errorCode: Int, errorMessage: String) {
        LogUtils.i(TAG, "onError $errorCode $errorMessage")
        Snackbar.make(gl_surface_view, errorMessage, Snackbar.LENGTH_SHORT).show()
    }

    override fun onProgressChanged(progressUs: Int) {
        gl_surface_view.seekTo(progressUs)
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

    }

    companion object {
        private const val TAG = "ExternalGPlayerViewActivity"
    }
}