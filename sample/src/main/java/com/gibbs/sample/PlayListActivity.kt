package com.gibbs.sample

import android.Manifest
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.provider.MediaStore
import android.view.*
import android.widget.TextView
import androidx.appcompat.widget.AppCompatImageView
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.core.app.ActivityCompat
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.bumptech.glide.Glide
import com.bumptech.glide.request.RequestOptions
import com.gibbs.gplayer.utils.LogUtils
import com.gibbs.sample.widget.DividerItemDecoration
import kotlinx.android.synthetic.main.activity_play_list.*
import java.util.*

class PlayListActivity : BaseActivity() {
    private var mAdapter: RecyclerView.Adapter<VideoItemHolder>? = null
    private val mVideoList: MutableList<VideoItem> = ArrayList()
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        supportActionBar?.setDisplayHomeAsUpEnabled(false)
        supportActionBar?.setDisplayShowHomeEnabled(false)
        setContentView(R.layout.activity_play_list)
        mAdapter = object : RecyclerView.Adapter<VideoItemHolder>() {
            override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): VideoItemHolder {
                val view = LayoutInflater.from(this@PlayListActivity).inflate(R.layout.item_video, parent, false)
                val holder: VideoItemHolder
                holder = VideoItemHolder(view)
                return holder
            }

            override fun onBindViewHolder(holder: VideoItemHolder, position: Int) {
                val videoItem = mVideoList[position]
                holder.videoName.text = videoItem.name
                Glide.with(this@PlayListActivity)
                        .applyDefaultRequestOptions(RequestOptions().centerCrop())
                        .load(videoItem.videoPath)
                        .into(holder.videoThumbnail)
                holder.rootView.setOnClickListener {
                    val style: String? = SettingsSPUtils.instance.getGPlayerStyle(this@PlayListActivity)
                    var intent = Intent(this@PlayListActivity, GPlayerActivity::class.java)
                    when (style) {
                        "simple" -> intent = Intent(this@PlayListActivity, SimpleGPlayerViewActivity::class.java)
                        "external" -> intent = Intent(this@PlayListActivity, ExternalGPlayerViewActivity::class.java)
                        "open_gl" -> intent = Intent(this@PlayListActivity, GPlayerActivity::class.java)
                    }
                    intent.putExtra("url", videoItem.videoPath)
                    val decodeSource: Boolean = SettingsSPUtils.instance.isDecodeSource(this@PlayListActivity)
                    val useMediaCodec: Boolean = SettingsSPUtils.instance.isMediaCodec(this@PlayListActivity)
                    intent.putExtra("decodeSource", decodeSource)
                    intent.putExtra("useMediaCodec", useMediaCodec)
                    LogUtils.i("PlayListActivity", "decodeSource = $decodeSource, useMediaCodec = $useMediaCodec")
                    startActivity(intent)
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
        LogUtils.i("PlayListActivity", "onRequestPermissionsResult requestCode = " + requestCode + ", permissions = " +
                permissions.contentToString() + ", grantResults = " + grantResults.contentToString())
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == REQUEST_READ_EXTERNAL_STORAGE_PERMISSION) {
            if (grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                mVideoList.addAll(getVideoFromSDCard(this@PlayListActivity))
            }
            mAdapter!!.notifyDataSetChanged()
        }
    }

    override fun onBackPressed() {}

    /**
     * 从SD卡得到所有的视频地址
     */
    private fun getVideoFromSDCard(context: Context): List<VideoItem> {
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

    private class VideoItem internal constructor(val videoPath: String, val name: String)

    private class VideoItemHolder internal constructor(itemView: View) : RecyclerView.ViewHolder(itemView) {
        val rootView: ConstraintLayout = itemView.findViewById(R.id.root_view)
        val videoThumbnail: AppCompatImageView = itemView.findViewById(R.id.video_thumbnail)
        val videoName: TextView = itemView.findViewById(R.id.video_name)
    }

    companion object {
        private const val REQUEST_READ_EXTERNAL_STORAGE_PERMISSION = 1
    }
}