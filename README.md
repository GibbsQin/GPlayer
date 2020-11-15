# GPlayer

[README English](README.en.md)

#### 介绍
GPlayer是一款基于 ffmpeg、mediacodec 的Android视频播放器框架。同时支持软解码和硬解码，通过OpenGL和AudioTrack进行渲染。

#### 软件架构
![](https://images.gitee.com/uploads/images/2020/0728/090047_bf03f1d8_5383286.png "components.png")

#### 下载
[![](https://www.jitpack.io/v/GibbsQin/GPlayer.svg)](https://www.jitpack.io/#GibbsQin/GPlayer)

	allprojects {
		repositories {
			...
			maven { url 'https://www.jitpack.io' }
		}
	}

	dependencies {
	        implementation 'com.github.GibbsQin:GPlayer:1.1.9'
	}

#### 使用
    <com.gibbs.gplayer.GPlayerView
        android:id="@+id/gl_surface_view"
        android:layout_width="0dp"
        android:layout_height="0dp"
        app:layout_constraintDimensionRatio="H,16:9"
        app:layout_constraintEnd_toEndOf="parent"
        app:layout_constraintStart_toStartOf="parent"
        app:layout_constraintTop_toTopOf="parent"
        app:layout_constraintBottom_toBottomOf="parent"/>

    private GPlayerView mVideoView;
    mVideoView = findViewById(R.id.gl_surface_view);
    mVideoView.setUrl(url);

    @Override
    protected void onResume() {
        super.onResume();
        if (mVideoView != null) {
            mVideoView.onResume();
            mVideoView.startPlay();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mVideoView != null) {
            mVideoView.onPause();
            mVideoView.stopPlay();
        }
    }