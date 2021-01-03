package com.gibbs.gplayer.sample

import android.Manifest
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.os.Handler
import android.provider.MediaStore
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.gibbs.gplayer.sample.model.PlayList
import com.gibbs.gplayer.sample.model.VideoItem
import java.io.InputStream

class MainActivity : AppCompatActivity() {
    private var mHandler: Handler = Handler()
    private val mVideoList: ArrayList<VideoItem> = ArrayList()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE),
                REQUEST_READ_EXTERNAL_STORAGE_PERMISSION)
    }

    override fun onDestroy() {
        super.onDestroy()
        mHandler.removeCallbacksAndMessages(null)
    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<String>, grantResults: IntArray) {
        LogUtils.i("MainActivity", "onRequestPermissionsResult requestCode = $requestCode," +
                " permissions = ${permissions.contentToString()}, grantResults = ${grantResults.contentToString()}")
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == REQUEST_READ_EXTERNAL_STORAGE_PERMISSION) {
            if (grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                val videoList = getVideoFromSDCard(this)
                getVideoFromWeb(videoList)
                mVideoList.addAll(videoList)
            }

            mHandler.postDelayed({
                startPlayerActivity()
                finish()
            }, 1000)
        }
    }

    private fun startPlayerActivity() {
        val intent = Intent(this@MainActivity, PlayListActivity::class.java)
        intent.putParcelableArrayListExtra("videoList", mVideoList)
        startActivity(intent)
    }

    /**
     * 从SD卡得到所有的视频地址
     */
    private fun getVideoFromSDCard(context: Context): MutableList<VideoItem> {
        val list: MutableList<VideoItem> = ArrayList()
        val projection = arrayOf(MediaStore.Video.Media.DATA, MediaStore.Video.Media.DISPLAY_NAME)
        val cursor = context.contentResolver.query(MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
                projection, null, null, null)
                ?: return list
        while (cursor.moveToNext()) {
            val path = cursor.getString(cursor.getColumnIndexOrThrow(MediaStore.Video.Media.DATA))
            val name = cursor.getString(cursor.getColumnIndexOrThrow(MediaStore.Video.Media.DISPLAY_NAME))
            val video = VideoItem(path, name)
            list.add(video)
        }
        cursor.close()
        return list
    }

    private fun getVideoFromWeb(list : MutableList<VideoItem>) {
        val playList = PlayList.createPlayList(getJsonString())
        for (link in playList.links) {
            list.add(VideoItem(link.url, link.name))
        }
    }

    private fun getJsonString(): String? {
        val fileName = "playlist.json"

        val inputStream: InputStream = assets.open(fileName)
        val size: Int = inputStream.available()
        val buffer = ByteArray(size)
        inputStream.read(buffer)
        inputStream.close()
        return String(buffer)
    }

    companion object {
        private const val REQUEST_READ_EXTERNAL_STORAGE_PERMISSION = 1
    }
}