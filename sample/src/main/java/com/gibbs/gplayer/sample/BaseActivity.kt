package com.gibbs.gplayer.sample

import android.os.Bundle
import android.view.MenuItem
import androidx.appcompat.app.AppCompatActivity

open class BaseActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        supportActionBar?.setDisplayHomeAsUpEnabled(true)
        supportActionBar?.setDisplayShowHomeEnabled(true)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        LogUtils.i("BaseActivity", "onOptionsItemSelected ${item.itemId}")
        if (item.itemId == android.R.id.home) {
            LogUtils.i("BaseActivity", "finish activity")
            finish()
            return true
        }
        return super.onOptionsItemSelected(item)
    }
}