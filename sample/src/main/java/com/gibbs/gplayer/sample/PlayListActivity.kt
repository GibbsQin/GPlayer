package com.gibbs.gplayer.sample

import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.LayoutInflater
import android.view.ViewGroup
import android.view.WindowManager
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.bumptech.glide.Glide
import com.bumptech.glide.request.RequestOptions
import com.gibbs.gplayer.GPlayer
import com.gibbs.gplayer.listener.OnPreparedListener
import com.gibbs.gplayer.listener.OnStateChangedListener
import com.gibbs.gplayer.sample.model.VideoItem
import com.gibbs.gplayer.sample.model.VideoItemHolder
import com.gibbs.gplayer.sample.widget.DividerItemDecoration
import com.gibbs.gplayer.utils.LogUtils
import kotlinx.android.synthetic.main.activity_play_list.*

class PlayListActivity : BaseActivity(), OnPreparedListener, OnStateChangedListener {
    private var mAdapter: RecyclerView.Adapter<VideoItemHolder>? = null
    private val mVideoList: ArrayList<VideoItem> = ArrayList()
    private var mPendingStart : Boolean = false
    private var mPendingUrl : String = ""

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        setContentView(R.layout.activity_play_list)
        gl_surface_view.setOnPreparedListener(this)
        gl_surface_view.setOnStateChangedListener(this)
        val url = intent.getStringExtra("url")
        val useMediaCodec = intent.getBooleanExtra("useMediaCodec", false)
        if (useMediaCodec) {
            gl_surface_view.setFlags(GPlayer.USE_MEDIA_CODEC)
        }
        gl_surface_view.setDataSource(url)

        mAdapter = object : RecyclerView.Adapter<VideoItemHolder>() {
            override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): VideoItemHolder {
                val view = LayoutInflater.from(this@PlayListActivity).inflate(R.layout.item_video, parent, false)
                return VideoItemHolder(view)
            }

            override fun onBindViewHolder(holder: VideoItemHolder, position: Int) {
                val videoItem = mVideoList[position]
                holder.videoName.text = videoItem.name
                Glide.with(this@PlayListActivity)
                        .applyDefaultRequestOptions(RequestOptions().centerCrop().error(R.drawable.ic_video_play))
                        .load(videoItem.videoPath)
                        .into(holder.videoThumbnail)
                holder.rootView.setOnClickListener {
                    mHandler.removeCallbacksAndMessages(null)
                    startPlay(videoItem)
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
        val videoList = intent.getParcelableArrayListExtra<VideoItem>("videoList")
        mVideoList.addAll(videoList!!)
        mAdapter!!.notifyDataSetChanged()
    }

    private fun startPlay(videoItem : VideoItem) {
        LogUtils.i("PlayListActivity", "startPlay $videoItem")
        if (gl_surface_view.isPlaying) {
            gl_surface_view.stop()
            mPendingStart = true
            mPendingUrl = videoItem.videoPath
        } else {
            gl_surface_view.setDataSource(videoItem.videoPath)
            gl_surface_view.prepare()
        }

        startAutoTest()
    }

    override fun onStart() {
        super.onStart()
        gl_surface_view.playerView.onResume()
        gl_surface_view.prepare()
    }

    override fun onStop() {
        super.onStop()
        gl_surface_view.playerView.onPause()
        gl_surface_view.stop()
        mHandler.removeCallbacksAndMessages(null)
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

    private val mHandler: Handler = Handler(Looper.getMainLooper())
    private fun startAutoTest() {
        val index = System.nanoTime() % mVideoList.size;
        mHandler.postDelayed(Runnable { startPlay(mVideoList[index.toInt()]) }, 10 * 1000)
    }
}