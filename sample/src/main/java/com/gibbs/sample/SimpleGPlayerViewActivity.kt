package com.gibbs.sample

import android.os.Bundle
import com.gibbs.gplayer.GPlayer
import com.gibbs.gplayer.listener.OnPreparedListener
import kotlinx.android.synthetic.main.activity_simple_gplayer.gl_surface_view

class SimpleGPlayerViewActivity : BaseActivity(), OnPreparedListener {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_simple_gplayer)
        val url = intent.getStringExtra("url")
        val useMediaCodec = intent.getBooleanExtra("useMediaCodec", false)
        if (useMediaCodec) {
            gl_surface_view.setFlags(GPlayer.USE_MEDIA_CODEC)
        }
        gl_surface_view.setDataSource(url)
        gl_surface_view.setOnPreparedListener(this)
    }

    override fun onStart() {
        super.onStart()
        gl_surface_view.onResume()
        gl_surface_view.prepare()
    }

    override fun onStop() {
        super.onStop()
        gl_surface_view.onPause()
        gl_surface_view.stop()
    }

    override fun onPrepared() {
        gl_surface_view.start()
    }
}