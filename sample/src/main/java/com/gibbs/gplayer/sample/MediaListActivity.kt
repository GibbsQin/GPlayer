package com.gibbs.gplayer.sample

import android.Manifest
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.provider.MediaStore
import android.view.*
import androidx.core.app.ActivityCompat
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.bumptech.glide.Glide
import com.bumptech.glide.request.RequestOptions
import com.gibbs.gplayer.utils.LogUtils
import com.gibbs.gplayer.sample.model.PlayList
import com.gibbs.gplayer.sample.model.VideoItem
import com.gibbs.gplayer.sample.model.VideoItemHolder
import com.gibbs.gplayer.sample.widget.DividerItemDecoration
import kotlinx.android.synthetic.main.activity_media_list.*
import java.io.InputStream


class MediaListActivity : BaseActivity() {
    private var mAdapter: RecyclerView.Adapter<VideoItemHolder>? = null
    private val mVideoList: ArrayList<VideoItem> = ArrayList()
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        supportActionBar?.setDisplayHomeAsUpEnabled(false)
        supportActionBar?.setDisplayShowHomeEnabled(false)
        setContentView(R.layout.activity_media_list)
        mAdapter = object : RecyclerView.Adapter<VideoItemHolder>() {
            override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): VideoItemHolder {
                val view = LayoutInflater.from(this@MediaListActivity).inflate(R.layout.item_video, parent, false)
                return VideoItemHolder(view)
            }

            override fun onBindViewHolder(holder: VideoItemHolder, position: Int) {
                val videoItem = mVideoList[position]
                holder.videoName.text = videoItem.name
                Glide.with(this@MediaListActivity)
                        .applyDefaultRequestOptions(RequestOptions().centerCrop().error(R.drawable.ic_video_play))
                        .load(videoItem.videoPath)
                        .into(holder.videoThumbnail)
                holder.rootView.setOnClickListener {
                    startPlayer(videoItem)
                }
            }

            override fun getItemCount(): Int {
                return mVideoList.size
            }
        }
        video_list.adapter = mAdapter
        video_list.layoutManager = LinearLayoutManager(this, RecyclerView.VERTICAL, false)
        val dividerItemDecoration = DividerItemDecoration(this, RecyclerView.VERTICAL)
        dividerItemDecoration.setMarginLeft(136)
        dividerItemDecoration.setMarginRight(24)
        video_list.addItemDecoration(dividerItemDecoration)
        ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE),
                REQUEST_READ_EXTERNAL_STORAGE_PERMISSION)
    }

    private fun startPlayer(videoItem : VideoItem) {
        val intent : Intent = when (SettingsSPUtils.instance.getGPlayerStyle(this@MediaListActivity)) {
            "simple" -> Intent(this@MediaListActivity, SimpleGPlayerViewActivity::class.java)
            "external" -> Intent(this@MediaListActivity, ExternalGPlayerViewActivity::class.java)
            "list" -> Intent(this@MediaListActivity, PlayListActivity::class.java)
            else -> Intent(this@MediaListActivity, SimpleGPlayerViewActivity::class.java)
        }
        intent.putExtra("url", videoItem.videoPath)
        intent.putExtra("name", videoItem.name)
        intent.putParcelableArrayListExtra("videoList", mVideoList)
        val useMediaCodec = SettingsSPUtils.instance.isMediaCodec(this@MediaListActivity)
        intent.putExtra("useMediaCodec", useMediaCodec)
        LogUtils.i("PlayListActivity", "useMediaCodec = $useMediaCodec")
        startActivity(intent)
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.setting_menu, menu)
        return super.onCreateOptionsMenu(menu)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        if (item.itemId == R.id.action_menu_setting) {
            startActivity(Intent(this, SettingsActivity::class.java))
            return true
        }
        return super.onOptionsItemSelected(item)
    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<String>, grantResults: IntArray) {
        LogUtils.i("PlayListActivity", "onRequestPermissionsResult requestCode = $requestCode," +
                " permissions = ${permissions.contentToString()}, grantResults = ${grantResults.contentToString()}")
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == REQUEST_READ_EXTERNAL_STORAGE_PERMISSION) {
            if (grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                val videoList = getVideoFromSDCard(this@MediaListActivity)
                getVideoFromWeb(videoList)
                mVideoList.addAll(videoList)
            }
            mAdapter!!.notifyDataSetChanged()
        }
    }

    override fun onBackPressed() {}

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