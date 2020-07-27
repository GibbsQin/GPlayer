package com.gibbs.gplayer.codec;

import android.media.MediaCodec;
import android.media.MediaFormat;
import android.view.Surface;

import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.render.AVSync;
import com.gibbs.gplayer.source.MediaSource;
import com.gibbs.gplayer.utils.LogUtils;
import com.gibbs.gplayer.utils.MediaExtractorUtils;

import java.nio.ByteBuffer;

public class MediaCodecVideoDecoder extends MediaCodecBaseDecoder implements VideoDecoder {
    private static final boolean USE_MEDIA_EXTRACTOR = true;

    private Surface mSurface;
    private AVSync mAVSync;

    public MediaCodecVideoDecoder(MediaSource MediaSource) {
        mMediaSource = MediaSource;
    }

    @Override
    String getMineByAVHeader(MediaInfo header) {
        return C.getVideoMineByAVHeader(header);
    }

    void configure(String mine) {
        int width = mAVHeader.getInteger(MediaInfo.KEY_WIDTH, 0);
        int height = mAVHeader.getInteger(MediaInfo.KEY_HEIGHT, 0);
        int frameRate = mAVHeader.getInteger(MediaInfo.KEY_FRAME_RATE, 20);

        MediaFormat formatFromExtractor = null;
        if (USE_MEDIA_EXTRACTOR && mMediaSource.getUrl().startsWith("file:")) {
            String url = mMediaSource.getUrl().replace("file:", "");
            formatFromExtractor = MediaExtractorUtils.getFormatByMine(url, mine);
        }
        MediaFormat format;
        if (formatFromExtractor != null) {
            format = formatFromExtractor;
        } else {
            format = MediaFormat.createVideoFormat(mine, width, height);
            format.setInteger(MediaFormat.KEY_FRAME_RATE, frameRate);
        }
        LogUtils.i(TAG, "input format = " + format.toString());
        codec.configure(format, mSurface, null, 0);
        LogUtils.i(TAG, "surface valid = " + (mSurface != null && mSurface.isValid()));
        mMediaFormat = codec.getOutputFormat();
        LogUtils.i(TAG, "output format = " + mMediaFormat.toString());
    }

    @Override
    MediaData readSource() {
        return mMediaSource.readVideoSource();
    }

    @Override
    void removeFirstPackage() {
        mMediaSource.removeFirstVideoPackage();
    }

    @Override
    void onOutputBuffer(int outputBufferId, ByteBuffer outputBuffer, MediaCodec.BufferInfo bufferInfo) {
        mOutputQueue.addLast(new MediaCodecOutputBuffer(null, outputBufferId, bufferInfo));
    }

    @Override
    public void setSurface(Surface surface) {
        mSurface = surface;
    }

    @Override
    public void setAVSync(AVSync avSync) {
        mAVSync = avSync;
    }

    @Override
    public void renderBuffer() {
        synchronized (mOutputQueueLock) {
            MediaCodecOutputBuffer element = mOutputQueue.peekFirst();
            if (element != null) {
                //AVSync
                long twiceVsyncDurationUs = 2 * mAVSync.getVsyncDurationNs() / 1000;
                long realTimeUs = mAVSync.getRealTimeUsForMediaTime(element.bufferInfo.presentationTimeUs); //映射到nowUs时间轴上
                realTimeUs -= twiceVsyncDurationUs; //提前两个VSync时间播放
                long lateUs = System.nanoTime() / 1000 - realTimeUs;

                if (lateUs < -twiceVsyncDurationUs) {
                    // too early;
                    return;
                }
                codec.releaseOutputBuffer(element.bufferId, true);
                mOutputQueue.removeFirst();
            }
        }
    }
}
