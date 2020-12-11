package com.gibbs.sample

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.WindowManager
import android.widget.TextView
import androidx.appcompat.widget.AppCompatImageView
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.gibbs.gplayer.GPlayer
import com.gibbs.gplayer.listener.OnPreparedListener
import com.gibbs.gplayer.listener.OnStateChangedListener
import com.gibbs.sample.widget.DividerItemDecoration
import kotlinx.android.synthetic.main.activity_play_list.*

class PlayListActivity : BaseActivity(), OnPreparedListener, OnStateChangedListener {
    private var mAdapter: RecyclerView.Adapter<VideoItemHolder>? = null
    private val mVideoList: MutableList<VideoItem> = ArrayList()
    private var mPendingStart : Boolean = false
    private var mPendingUrl : String = ""

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        setContentView(R.layout.activity_play_list)
        gl_surface_view.setOnPreparedListener(this)
        gl_surface_view.setOnStateChangedListener(this)
        val url = intent.getStringExtra("url")
        gl_surface_view.setDataSource(url)

        mAdapter = object : RecyclerView.Adapter<VideoItemHolder>() {
            override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): VideoItemHolder {
                val view = LayoutInflater.from(this@PlayListActivity).inflate(R.layout.item_video, parent, false)
                return VideoItemHolder(view)
            }

            override fun onBindViewHolder(holder: VideoItemHolder, position: Int) {
                val videoItem = mVideoList[position]
                holder.videoName.text = videoItem.name
                holder.rootView.setOnClickListener {
                    if (gl_surface_view.isPlaying) {
                        gl_surface_view.stop()
                        mPendingStart = true
                        mPendingUrl = videoItem.videoPath
                    } else {
                        gl_surface_view.setDataSource(videoItem.videoPath)
                        gl_surface_view.prepare()
                    }
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
        mVideoList.add(VideoItem("http://cctvalih5ca.v.myalicdn.com/live/cctv1_2/index.m3u8", "cctv1"))
        mVideoList.add(VideoItem("http://cctvalih5ca.v.myalicdn.com/live/cctv2_2/index.m3u8", "cctv2"))
        mVideoList.add(VideoItem("http://cctvalih5ca.v.myalicdn.com/live/cctv3_2/index.m3u8", "cctv3"))
        mAdapter!!.notifyDataSetChanged()
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

    override fun onStateChanged(state: GPlayer.State?) {
        if (state == GPlayer.State.STOPPED) {
            if (mPendingStart) {
                mPendingStart = false
                gl_surface_view.setDataSource(mPendingUrl)
                gl_surface_view.prepare()
            }
        }
    }

    private class VideoItem internal constructor(val videoPath: String, val name: String)

    private class VideoItemHolder internal constructor(itemView: View) : RecyclerView.ViewHolder(itemView) {
        val rootView: ConstraintLayout = itemView.findViewById(R.id.root_view)
        val videoThumbnail: AppCompatImageView = itemView.findViewById(R.id.video_thumbnail)
        val videoName: TextView = itemView.findViewById(R.id.video_name)
    }
}