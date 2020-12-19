package com.gibbs.gplayer.sample.model

import android.view.View
import android.widget.TextView
import androidx.appcompat.widget.AppCompatImageView
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.recyclerview.widget.RecyclerView
import com.gibbs.gplayer.sample.R

class VideoItemHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
    val rootView: ConstraintLayout = itemView.findViewById(R.id.root_view)
    val videoThumbnail: AppCompatImageView = itemView.findViewById(R.id.video_thumbnail)
    val videoName: TextView = itemView.findViewById(R.id.video_name)
}