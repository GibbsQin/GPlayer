package com.gibbs.sample

import android.os.Bundle
import com.gibbs.gplayer.listener.OnPreparedListener
import kotlinx.android.synthetic.main.activity_simple_gplayer.*

class SimpleGPlayerViewActivity : BaseActivity(), OnPreparedListener {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_simple_gplayer)
        val url = intent.getStringExtra("url")
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