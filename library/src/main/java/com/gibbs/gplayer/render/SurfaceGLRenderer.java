package com.gibbs.gplayer.render;

import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.view.Surface;

import com.gibbs.gplayer.codec.MediaCodecVideoDecoder;
import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.codec.VideoDecoder;
import com.gibbs.gplayer.source.MediaSource;
import com.gibbs.gplayer.utils.LogUtils;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class SurfaceGLRenderer extends BaseVideoRender
        implements SurfaceTexture.OnFrameAvailableListener {
    private static final String TAG = "SurfaceGLRenderer";

    private VideoDecoder mVideoDecoder;
    private SurfaceGLProgram mSurfaceGLProgram;
    private SurfaceTexture mSurfaceTexture;
    private boolean updateSurface = false;

    public SurfaceGLRenderer(GLSurfaceView view, MediaSource source) {
        super(source);
        mVideoDecoder = new MediaCodecVideoDecoder(source);
        mSurfaceGLProgram = new SurfaceGLProgram();
    }

    public SurfaceTexture getSurfaceTexture() {
        return mSurfaceTexture;
    }

    @Override
    public void onDrawFrame(GL10 glUnused) {
        synchronized (this) {
            if (updateSurface) {
                mSurfaceTexture.updateTexImage();
                updateSurface = false;
            }
        }

        mSurfaceGLProgram.drawFrame(mSurfaceTexture);
    }

    @Override
    public void onSurfaceChanged(GL10 glUnused, int width, int height) {
        super.onSurfaceChanged(glUnused, width, height);
        LogUtils.d(TAG, "onSurfaceChanged width = " + width + " height = " + height);
        //默认充满surface
        GLES20.glViewport(0, 0, width, height);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        LogUtils.e(TAG, "VideoRender surfaceCreated");
        mSurfaceGLProgram.surfaceCreated();

        /*
         * Create the SurfaceTexture that will feed this textureID,
         * and pass it to the Player
         */
        mSurfaceTexture = new SurfaceTexture(mSurfaceGLProgram.getTextureId());
        mSurfaceTexture.setOnFrameAvailableListener(this);
        mVideoDecoder.setSurface(new Surface(mSurfaceTexture));

        synchronized (this) {
            updateSurface = false;
        }
        super.onSurfaceCreated(gl, config);
    }

    @Override
    synchronized public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        updateSurface = true;
    }

    @Override
    public void init(MediaInfo mediaInfo) {
        mVideoDecoder.init(mediaInfo);
    }

    @Override
    public void render() {
        mVideoDecoder.renderBuffer();
    }

    @Override
    public void release() {
        super.release();
        mVideoDecoder.release();
    }

    @Override
    public void setAVSync(AVSync avSync) {
        super.setAVSync(avSync);
        mVideoDecoder.setAVSync(mAVSync);
    }

    @Override
    public void updateMvp(float[] mvp) {
        mSurfaceGLProgram.updateMvp(mvp);
    }
}
