package com.gibbs.sample

import android.os.Bundle
import com.gibbs.gplayer.source.MediaSource
import kotlinx.android.synthetic.main.activity_simple_g_player.*

class SimpleGPlayerViewActivity : BaseActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_simple_g_player)
        val url = intent.getStringExtra("url")
        gl_surface_view.setUrl(MediaSource.SOURCE_TYPE_FILE, url)
    }

    override fun onResume() {
        super.onResume()
        gl_surface_view?.onResume()
        gl_surface_view?.startPlay()
    }

    override fun onPause() {
        super.onPause()
        gl_surface_view?.onPause()
        gl_surface_view?.stopPlay()
    }
}