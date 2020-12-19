package com.gibbs.gplayer.sample

import android.os.Bundle
import com.gibbs.gplayer.GPlayer
import kotlinx.android.synthetic.main.activity_simple_gplayer.*

class SimpleGPlayerViewActivity : BaseActivity(), GPlayer.OnPreparedListener {
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
        gl_surface_view.prepare()
    }

    override fun onStop() {
        super.onStop()
        gl_surface_view.stop()
    }

    override fun onPrepared() {
        gl_surface_view.start()
    }
}