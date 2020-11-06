package com.gibbs.gplayer.render;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;

import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.source.MediaSource;
import com.gibbs.gplayer.utils.LogUtils;

import java.nio.ByteBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class YUVGLRenderer extends BaseVideoRender {
    private final String TAG = "YUVGLRenderer";

    private static final boolean VERBOSE = false;

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
        super(source);
        mSurfaceView = view;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        super.onSurfaceCreated(gl, config);
        LogUtils.d(TAG, "YUVGLRenderer :: onSurfaceCreated");
        mYUVGLProgram.buildProgram();
        LogUtils.d(TAG, "YUVGLRenderer :: b" + "uildProgram done");
        super.onSurfaceCreated(gl, config);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        super.onSurfaceChanged(gl, width, height);
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
            mSurfaceView.queueEvent(new Runnable() {
                @Override
                public void run() {
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
            });
        }
    }

    public void onVideoSizeChanged(int width, int height) {
        updateVideoSize(width, height);
        mSurfaceView.requestRender();
    }

    @Override
    public void init(MediaInfo header) {
        super.init(header);
        int width = header.getInteger(MediaInfo.KEY_WIDTH, 0);
        int height = header.getInteger(MediaInfo.KEY_HEIGHT, 0);
        updateVideoSize(width, height);
        mYUVGLProgram.updateRotate(header.getInteger(MediaInfo.KEY_VIDEO_ROTATE, 0));
        mInitialized = true;
        LogUtils.d(TAG, "onInit");
    }

    @Override
    public void render() {
        if (!mInitialized) {
            return;
        }

        MediaData data = mMediaSource.readVideoSource();
        if (data == null) {
            return;
        }

        //AVSync
        long twiceVsyncDurationUs = 2 * mAVSync.getVsyncDurationNs() / 1000;
        long realTimeUs = mAVSync.getRealTimeUsForMediaTime(data.pts); //映射到nowUs时间轴上
        realTimeUs -= twiceVsyncDurationUs; //提前两个VSync时间播放
        long lateUs = System.nanoTime() / 1000 - realTimeUs;

        if (lateUs < -twiceVsyncDurationUs) {
            // too early;
            if (mFrameIntervalMs < mMaxFrameIntervalMs - INCREMENT_INTERVAL_MS) {
                mFrameIntervalMs += INCREMENT_INTERVAL_MS;
            }
            try {
                if (VERBOSE) LogUtils.i(TAG, "sleep for " + mFrameIntervalMs + " ms");
                Thread.sleep(mFrameIntervalMs);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            return;
        } else {
            if (mFrameIntervalMs > INCREMENT_INTERVAL_MS) {
                mFrameIntervalMs -= INCREMENT_INTERVAL_MS;
            }
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
        mMediaSource.removeFirstVideoPackage();

        try {
            if (VERBOSE) LogUtils.i(TAG, "sleep for " + mFrameIntervalMs + " ms");
            Thread.sleep(mFrameIntervalMs);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void release() {
        super.release();
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

    @Override
    public void updateMvp(float[] mvp) {
        mYUVGLProgram.updateMvp(mvp);
        //update render immediately
        mSurfaceView.requestRender();
    }
}
