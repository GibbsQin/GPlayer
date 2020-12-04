package com.gibbs.sample

import android.os.Bundle
import kotlinx.android.synthetic.main.activity_simple_gplayer.*

class SimpleGPlayerViewActivity : BaseActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_simple_gplayer)
        val url = intent.getStringExtra("url")
        gl_surface_view.setDataSource(url)
    }

    override fun onResume() {
        super.onResume()
        gl_surface_view?.onResume()
        gl_surface_view?.prepare()
    }

    override fun onPause() {
        super.onPause()
        gl_surface_view?.onPause()
        gl_surface_view?.stop()
    }
}