package com.gibbs.sample;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.os.Bundle;
import android.provider.MediaStore;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.widget.AppCompatImageView;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.core.app.ActivityCompat;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.bumptech.glide.Glide;
import com.bumptech.glide.request.RequestOptions;
import com.gibbs.gplayer.utils.LogUtils;
import com.gibbs.sample.widget.DividerItemDecoration;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class PlayListActivity extends BaseActivity {
    private static final int REQUEST_READ_EXTERNAL_STORAGE_PERMISSION = 1;

    private RecyclerView.Adapter<VideoItemHolder> mAdapter;
    private List<VideoItem> mVideoList = new ArrayList<>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.setDisplayHomeAsUpEnabled(false);
            actionBar.setDisplayShowHomeEnabled(false);
        }
        setContentView(R.layout.activity_play_list);
        RecyclerView videoRecyclerView = findViewById(R.id.video_list);
        mAdapter = new RecyclerView.Adapter<VideoItemHolder>() {
            @NonNull
            @Override
            public VideoItemHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
                View view = LayoutInflater.from(PlayListActivity.this).inflate(R.layout.item_video, parent, false);
                VideoItemHolder holder;
                holder = new VideoItemHolder(view);
                return holder;
            }

            @Override
            public void onBindViewHolder(@NonNull VideoItemHolder holder, final int position) {
                final VideoItem videoItem = mVideoList.get(position);
                holder.videoName.setText(videoItem.name);
                Glide.with(PlayListActivity.this)
                        .applyDefaultRequestOptions(new RequestOptions().centerCrop())
                        .load(videoItem.path)
                        .into(holder.videoThumbnail);
                holder.rootView.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        String style = SettingsSPUtils.getInstance().getGPlayerStyle(PlayListActivity.this);
                        Intent intent = new Intent(PlayListActivity.this, GPlayerActivity.class);
                        switch (style) {
                            case "simple":
                                intent = new Intent(PlayListActivity.this, SimpleGPlayerViewActivity.class);
                                break;
                            case "external":
                                intent = new Intent(PlayListActivity.this, ExternalGPlayerViewActivity.class);
                                break;
                            case "open_gl":
                                intent = new Intent(PlayListActivity.this, GPlayerActivity.class);
                                break;
                        }
                        intent.putExtra("url", videoItem.getVideoPath());
                        boolean decodeSource = SettingsSPUtils.getInstance().isDecodeSource(PlayListActivity.this);
                        boolean useMediaCodec = SettingsSPUtils.getInstance().isMediaCodec(PlayListActivity.this);
                        intent.putExtra("decodeSource", decodeSource);
                        intent.putExtra("useMediaCodec", useMediaCodec);
                        LogUtils.i("PlayListActivity", "decodeSource = " + decodeSource + ", useMediaCodec = " + useMediaCodec);
                        startActivity(intent);
                    }
                });
            }

            @Override
            public int getItemCount() {
                return mVideoList.size();
            }
        };
        videoRecyclerView.setAdapter(mAdapter);
        videoRecyclerView.setLayoutManager(new LinearLayoutManager(this, RecyclerView.VERTICAL, false));
        DividerItemDecoration dividerItemDecoration = new DividerItemDecoration(this, RecyclerView.VERTICAL);
        dividerItemDecoration.setMarginLeft(136);
        dividerItemDecoration.setMarginRight(24);
        videoRecyclerView.addItemDecoration(dividerItemDecoration);
        ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE},
                REQUEST_READ_EXTERNAL_STORAGE_PERMISSION);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.setting_menu, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        if (item.getItemId() == R.id.action_menu_setting) {
            startActivity(new Intent(this, SettingsActivity.class));
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        LogUtils.i("PlayListActivity", "onRequestPermissionsResult requestCode = " + requestCode + ", permissions = " +
                Arrays.toString(permissions) + ", grantResults = " + Arrays.toString(grantResults));
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_READ_EXTERNAL_STORAGE_PERMISSION) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                mVideoList.addAll(getVideoFromSDCard(PlayListActivity.this));
            }
            mAdapter.notifyDataSetChanged();
        }
    }

    @Override
    public void onBackPressed() {

    }

    /**
     * 从SD卡得到所有的视频地址
     */
    private List<VideoItem> getVideoFromSDCard(Context context) {
        List<VideoItem> list = new ArrayList<>();
        String[] projection = new String[]{MediaStore.Video.Media.DATA, MediaStore.Video.Media.DISPLAY_NAME};
        Cursor cursor = context.getContentResolver().query(MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
                projection, null, null, null);
        if (cursor == null) {
            return list;
        }
        while (cursor.moveToNext()) {
            String path = cursor.getString(cursor.getColumnIndexOrThrow(MediaStore.Video.Media.DATA));
            String name = cursor.getString(cursor.getColumnIndexOrThrow(MediaStore.Video.Media.DISPLAY_NAME));
            VideoItem video = new VideoItem(path, name);
            list.add(video);
        }
        cursor.close();
        return list;
    }

    private static class VideoItem {
        private String path;
        private String name;

        VideoItem(String path, String name) {
            this.path = path;
            this.name = name;
        }

        private String getVideoPath() {
            return path;
        }
    }

    private static class VideoItemHolder extends RecyclerView.ViewHolder {
        private ConstraintLayout rootView;
        private AppCompatImageView videoThumbnail;
        private TextView videoName;

        VideoItemHolder(@NonNull View itemView) {
            super(itemView);
            rootView = itemView.findViewById(R.id.root_view);
            videoThumbnail = itemView.findViewById(R.id.video_thumbnail);
            videoName = itemView.findViewById(R.id.video_name);
        }
    }
}
