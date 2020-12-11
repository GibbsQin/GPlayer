package com.gibbs.gplayer.render;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.view.SurfaceView;

import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.source.MediaSource;
import com.gibbs.gplayer.utils.LogUtils;

import java.nio.ByteBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class YUVGLRenderer implements VideoRender {
    private final String TAG = "YUVGLRenderer";

    private static final boolean VERBOSE = false;

    private MediaSource mMediaSource;
    private GLSurfaceView mSurfaceView;
    private YUVGLProgram mYUVGLProgram = new YUVGLProgram();
    private ByteBuffer mByteBufferY = null;
    private ByteBuffer mByteBufferU = null;
    private ByteBuffer mByteBufferV = null;
    private volatile boolean mInitialized = false;

    /**
     * 视频帧宽高
     */
    private int mVideoWidth = 0;
    private int mVideoHeight = 0;

    private volatile boolean mIsFrameReady = false;

    public YUVGLRenderer(GLSurfaceView view, MediaSource source) {
        mMediaSource = source;
        mSurfaceView = view;
    }

    public static VideoRender createVideoRender(SurfaceView view, MediaSource mediaSource) {
        VideoRender videoRender = null;
        if (view instanceof GLSurfaceView) {
            GLSurfaceView glSurfaceView = (GLSurfaceView) view;
            videoRender = new YUVGLRenderer(glSurfaceView, mediaSource);
            glSurfaceView.setEGLContextClientVersion(2);
            glSurfaceView.setRenderer(videoRender);
            glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        }

        return videoRender;
    }


    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        LogUtils.d(TAG, "YUVGLRenderer :: onSurfaceCreated");
        mYUVGLProgram.buildProgram();
        LogUtils.d(TAG, "YUVGLRenderer :: b" + "uildProgram done");
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        LogUtils.d(TAG, "YUVGLRenderer :: onSurfaceChanged width = " + width + " height = " + height);
        //默认充满surface
        GLES20.glViewport(0, 0, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        //更新纹理
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        if (mIsFrameReady) {
            mYUVGLProgram.drawFrame();
        } else {
            if (mByteBufferY != null && mByteBufferU != null && mByteBufferV != null) {
                synchronized (YUVGLRenderer.this) {
                    mByteBufferY.position(0);
                    mByteBufferU.position(0);
                    mByteBufferV.position(0);
                    mYUVGLProgram.buildTextures(mByteBufferY, mByteBufferU, mByteBufferV, mVideoWidth, mVideoHeight);
                    mIsFrameReady = true;
                }
            }
        }
    }

    public void onVideoSizeChanged(int width, int height) {
        updateVideoSize(width, height);
        mSurfaceView.requestRender();
    }

    @Override
    public void init(MediaSource mediaSource) {
        int width = mediaSource.getWidth();
        int height = mediaSource.getHeight();
        updateVideoSize(width, height);
        mYUVGLProgram.updateRotate(mediaSource.getRotate());
        mInitialized = true;
        LogUtils.d(TAG, "onInit");
    }

    @Override
    public long render() {
        if (!mInitialized) {
            return 0;
        }

        MediaData data = mMediaSource.readVideoSource();
        if (data == null) {
            return 0;
        }

        if (VERBOSE) LogUtils.d(TAG, "render video : " + data.pts);

        synchronized (YUVGLRenderer.this) {
            mByteBufferY = data.data;
            mByteBufferU = data.data1;
            mByteBufferV = data.data2;
        }

        mSurfaceView.queueEvent(new Runnable() {
            @Override
            public void run() {

                synchronized (YUVGLRenderer.this) {
                    if (mByteBufferY != null && mByteBufferU != null && mByteBufferV != null) {
                        mByteBufferY.position(0);
                        mByteBufferU.position(0);
                        mByteBufferV.position(0);
                        mYUVGLProgram.buildTextures(mByteBufferY, mByteBufferU, mByteBufferV, mVideoWidth, mVideoHeight);
                        mIsFrameReady = true;
                    }
                }

            }
        });

        // request to render
        mSurfaceView.requestRender();
        long renderTime = data.pts;
        mMediaSource.removeFirstVideoPackage();
        return renderTime;
    }

    @Override
    public void release() {
        mInitialized = false;
        mIsFrameReady = false;
        mByteBufferY = null;
        mByteBufferU = null;
        mByteBufferV = null;
    }

    private void updateVideoSize(int width, int height) {
        if (width != mVideoWidth || height != mVideoHeight) {
            this.mVideoWidth = width;
            this.mVideoHeight = height;
        }
    }
}
