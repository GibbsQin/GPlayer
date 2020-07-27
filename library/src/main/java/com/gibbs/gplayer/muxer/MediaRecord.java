package com.gibbs.gplayer.muxer;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaMuxer;

import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.codec.C;
import com.gibbs.gplayer.codec.MediaCodecAudioEncoder;
import com.gibbs.gplayer.codec.MediaCodecVideoEncoder;
import com.gibbs.gplayer.source.MediaSource;
import com.gibbs.gplayer.utils.LogUtils;

import java.io.IOException;
import java.nio.ByteBuffer;

public class MediaRecord {
    private static final String TAG = "MediaRecord";

    private MediaCodecAudioEncoder mAudioEncoder;
    private MediaCodecVideoEncoder mVideoEncoder;
    private MediaData mAudioData;
    private MediaData mVideoData;
    private MediaCodec.BufferInfo mAudioBufferInfo = new MediaCodec.BufferInfo();
    private MediaCodec.BufferInfo mVideoBufferInfo = new MediaCodec.BufferInfo();
    private MediaMuxer mMuxer;
    private MediaSource mMediaSource;
    private int mAudioTrackIndex = -1;
    private int mVideoTrackIndex = -1;
    private MuxerThread mMuxerThread;
    private boolean mIsMuxing;

    public MediaRecord(String filePath, MediaSource mediaSource) {
        if ((mediaSource.getFlag() & MediaSource.FLAG_DECODE) != MediaSource.FLAG_DECODE) {
            throw new IllegalArgumentException("must be decode MediaSource");
        }
        try {
            mMuxer = new MediaMuxer(filePath, MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4);
        } catch (IOException e) {
            e.printStackTrace();
            return;
        }
        mMediaSource = mediaSource;
    }

    private void newAudioEncode() {
        if (mMuxer == null) {
            LogUtils.e(TAG, "addAudioTrack fail due to null muxer");
            return;
        }
        mAudioEncoder = new MediaCodecAudioEncoder(false);
        MediaInfo header = mMediaSource.getMediaInfo();
        int sampleNumFrame = header.getInteger(MediaInfo.KEY_AUDIO_SAMPLE_NUM_PERFRAME, 1024);
        int channels = header.getInteger(MediaInfo.KEY_AUDIO_CHANNELS, 1);
        mAudioEncoder.init(header);
        mAudioData = new MediaData();
        mAudioData.data = ByteBuffer.allocate(sampleNumFrame * channels * 2);
    }

    private void newVideoEncode() {
        if (mMuxer == null) {
            LogUtils.e(TAG, "addVideoTrack fail due to null muxer");
            return;
        }
        mVideoEncoder = new MediaCodecVideoEncoder(
                MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar, 300000);
        MediaInfo header = mMediaSource.getMediaInfo();
        int width = header.getInteger(MediaInfo.KEY_WIDTH, 0);
        int height = header.getInteger(MediaInfo.KEY_HEIGHT, 0);
        mVideoEncoder.init(header);
        mVideoData = new MediaData();
        mVideoData.data = ByteBuffer.allocate(width * height);
    }

    public void startMuxer() {
        if (mMuxer == null) {
            LogUtils.e(TAG, "startMuxer fail due to null muxer");
            return;
        }
        LogUtils.i(TAG, "startMuxer");
        mIsMuxing = true;
        mMuxerThread = new MuxerThread();
        mMuxerThread.start();
    }

    private void writeAudioSampleData(MediaData mediaData) {
        if (mMuxer == null) {
            LogUtils.e(TAG, "writeAudioSampleData fail due to null muxer");
            return;
        }
        int result = mAudioEncoder.send_frame(mediaData);
        if (result == C.MEDIA_CODEC_SUCCESS) {
            mMediaSource.removeFirstAudioPackage();
        }
        result = mAudioEncoder.receive_packet(mAudioData, mAudioBufferInfo);
        if (result == C.MEDIA_CODEC_SUCCESS && mAudioTrackIndex >= 0 && mVideoTrackIndex >= 0) {
            mMuxer.writeSampleData(mAudioTrackIndex, mAudioData.data, mAudioBufferInfo);
            LogUtils.d(TAG, "writeAudioSampleData " + mAudioBufferInfo.size);
        } else if (result == C.MEDIA_CODEC_FORMAT_CHANGED) {
            mAudioTrackIndex = mMuxer.addTrack(mAudioEncoder.getMediaFormat());
            LogUtils.i(TAG, "addAudioTrack = " + mAudioTrackIndex);
            if (mVideoTrackIndex >= 0) {
                mMuxer.start();
            }
        }
    }

    private void writeVideoSampleData(MediaData mediaData) {
        if (mMuxer == null) {
            LogUtils.e(TAG, "writeVideoSampleData fail due to null muxer");
            return;
        }
        int result = mVideoEncoder.send_frame(mediaData);
        if (result == C.MEDIA_CODEC_SUCCESS) {
            mMediaSource.removeFirstVideoPackage();
        }
        result = mVideoEncoder.receive_packet(mVideoData, mVideoBufferInfo);
        if (result == C.MEDIA_CODEC_SUCCESS && mAudioTrackIndex >= 0 && mVideoTrackIndex >= 0) {
            mMuxer.writeSampleData(mVideoTrackIndex, mVideoData.data, mVideoBufferInfo);
        } else if (result == C.MEDIA_CODEC_FORMAT_CHANGED) {
            mVideoTrackIndex = mMuxer.addTrack(mVideoEncoder.getMediaFormat());
            LogUtils.i(TAG, "addVideoTrack = " + mVideoTrackIndex);
            if (mAudioTrackIndex >= 0) {
                mMuxer.start();
            }
        }
    }

    public void stopMuxer() {
        mIsMuxing = false;
        if (mMuxerThread != null && mMuxerThread.isAlive()) {
            try {
                mMuxerThread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        LogUtils.i(TAG, "stopMuxer");
    }

    private class MuxerThread extends Thread {
        @Override
        public void run() {
            super.run();
            LogUtils.i(TAG, "MuxerThread init " + getId());
            newAudioEncode();
            newVideoEncode();
            while (mIsMuxing) {
                MediaData audioData = mMediaSource.readAudioSource();
                if (audioData != null) {
                    writeAudioSampleData(audioData);
                }
                MediaData videoData = mMediaSource.readVideoSource();
                if (videoData != null) {
                    writeVideoSampleData(videoData);
                }
            }
            mMuxer.stop();
            mMuxer.release();
            mMuxer = null;
            LogUtils.i(TAG, "MuxerThread destroy " + getId());
        }
    }
}
