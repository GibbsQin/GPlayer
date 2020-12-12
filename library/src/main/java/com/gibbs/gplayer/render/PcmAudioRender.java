package com.gibbs.gplayer.render;

import android.media.AudioManager;
import android.media.AudioTrack;

import androidx.annotation.NonNull;

import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.source.MediaSource;
import com.gibbs.gplayer.utils.LogUtils;

import java.nio.ByteBuffer;

public class PcmAudioRender implements AudioRender {
    private static final String TAG = "PcmAudioRender";

    private static final boolean VERBOSE = false;

    private MediaSource mMediaSource;
    private AudioTrack mAudioTrack;

    public PcmAudioRender(MediaSource source) {
        mMediaSource = source;
    }

    @Override
    public synchronized void init(MediaSource mediaSource) {
        int sampleRate = mediaSource.getSampleRate();
        int channelLayout = (int) mediaSource.getChannelLayout();
        int format = mediaSource.getSampleFormat();
        int bytesPerSample = mediaSource.getBytesPerFrame();
        openAudioTrack(sampleRate, format, channelLayout, bytesPerSample);
    }

    @Override
    public long render() {
        MediaData mediaData = mMediaSource.readAudioSource();
        if (mediaData != null && mediaData.size > 0) {
            write(mediaData.data, mediaData.size, mediaData.pts, AudioTrack.WRITE_BLOCKING);
            long renderTime = mediaData.pts;
            mMediaSource.removeFirstAudioPackage();
            return renderTime;
        }
        return 0;
    }

    @Override
    public synchronized void release() {
        LogUtils.i(TAG, "release");
        stopAudioTrack();
    }

    @Override
    public int write(@NonNull ByteBuffer outputBuffer, int size, long presentationTimeUs, int writeMode) {
        int result;
        result = mAudioTrack.write(outputBuffer, size, writeMode);
        if (VERBOSE)
            LogUtils.d(TAG, "render audio " + presentationTimeUs + " " + size + " " + result);
        return result;
    }

    private void openAudioTrack(int sampleRate, int sampleFormat, int channelLayout, int bytesPerSample) {
        try {
            int minBufferSize = AudioTrack.getMinBufferSize(sampleRate, channelLayout, sampleFormat);
            int bufferSizeInBytes = Math.round(minBufferSize * 1.0f / bytesPerSample + 0.5f) * bytesPerSample;
            mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate, channelLayout,
                    sampleFormat, minBufferSize * 2, AudioTrack.MODE_STREAM);
            LogUtils.i(TAG, "openAudioTrack min buffer size:" + minBufferSize + ", bufferSizeInBytes:" + bufferSizeInBytes);
            mAudioTrack.play();
        } catch (Exception e) {
            LogUtils.e(TAG, "create AudioTrack error: " + e.getMessage());
        }
    }

    private void stopAudioTrack() {
        if (mAudioTrack != null) {
            mAudioTrack.flush();
            mAudioTrack.stop();
            mAudioTrack.release();
            mAudioTrack = null;
            LogUtils.i(TAG, "stopAudioTrack");
        }
    }
}
