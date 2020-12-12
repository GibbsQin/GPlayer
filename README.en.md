# GPlayer

#### Introduce
GPlayer is a video player frame base on ffmpeg and mediacodec。GPlayer support software decoding and hardware decoding, then render via AudioTrack and OpenGL。

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

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_simple_gplayer)
        val url = intent.getStringExtra("url")
        gl_surface_view.setDataSource(url)
        gl_surface_view.setOnPreparedListener(this)
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

