# GPlayer

#### Introduce
GPlayer is a video player frame base on ffmpeg and mediacodec。GPlayer support software decoding and hardware decoding, then render via AudioTrack and OpenGL。

#### Software architecture
![](https://images.gitee.com/uploads/images/2020/0727/182413_fc543975_5383286.png "components.png")

#### Download
[![](https://www.jitpack.io/v/GibbsQin/GPlayer.svg)](https://www.jitpack.io/#GibbsQin/GPlayer)

	allprojects {
		repositories {
			...
			maven { url 'https://www.jitpack.io' }
		}
	}

	dependencies {
	        implementation 'com.github.GibbsQin:GPlayer:+
	}

#### Usage
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

