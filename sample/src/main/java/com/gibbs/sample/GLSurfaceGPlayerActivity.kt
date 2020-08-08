package com.gibbs.sample

import android.os.Bundle
import com.gibbs.gplayer.GPlayer
import com.gibbs.gplayer.source.MediaSource
import com.gibbs.gplayer.utils.LogUtils
import kotlinx.android.synthetic.main.activity_glsurface_gplayer.*

class GLSurfaceGPlayerActivity : BaseActivity() {
    private var mGPlayer: GPlayer? = null
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_glsurface_gplayer)
        val decodeSource = intent.getBooleanExtra("decodeSource", false)
        val useMediaCodec = intent.getBooleanExtra("useMediaCodec", false)
        val url = intent.getStringExtra("url")
        LogUtils.i(TAG, "url = $url")
        LogUtils.i(TAG, "decodeSource = $decodeSource, useMediaCodec = $useMediaCodec")
        mGPlayer = GPlayer(gl_surface_view, MediaSource.SOURCE_TYPE_FILE, url, decodeSource, useMediaCodec)
    }

    override fun onResume() {
        LogUtils.i(TAG, "onResume")
        super.onResume()
        gl_surface_view.onResume()
        mGPlayer?.prepare()
    }

    override fun onPause() {
        LogUtils.i(TAG, "onPause")
        super.onPause()
        gl_surface_view.onPause()
        mGPlayer?.finish()
    }

    companion object {
        private const val TAG = "GPlayerActivity"
    }
}