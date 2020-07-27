package com.gibbs.gplayer.render;

import android.media.AudioManager;
import android.media.AudioTimestamp;
import android.media.AudioTrack;
import android.media.PlaybackParams;
import android.os.Build;

import androidx.annotation.NonNull;

import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.source.MediaSource;
import com.gibbs.gplayer.utils.LogUtils;

import java.lang.reflect.Method;
import java.nio.ByteBuffer;

abstract class BaseAudioRender implements AudioRender {
    private static final String TAG = "BaseAudioRender";

    private static final boolean VERBOSE = false;

    private static final int MIN_TIMESTAMP_SAMPLE_INTERVAL_US = 250_000;

    MediaSource mMediaSource;
    private AudioTrack mAudioTrack;

    private int mSampleRate;
    private int mSampleFormat;

    private Method getLatencyMethod;
    private long mLatencyUs;
    private long mLastTimestampSampleTimeUs;
    private boolean mAudioTimestampSet;
    private final AudioTimestamp mAudioTimestamp;

    BaseAudioRender(MediaSource source) {
        mMediaSource = source;
        try {
            getLatencyMethod =
                    android.media.AudioTrack.class.getMethod("getLatency", (Class<?>[]) null);
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        }
        mLatencyUs = 0;
        mLastTimestampSampleTimeUs = 0;
        mAudioTimestamp = new AudioTimestamp();
    }

    @Override
    public void init(MediaInfo mediaInfo) {
        LogUtils.i(TAG, "CoreFlow : init");
        mSampleRate = mediaInfo.getInteger(MediaInfo.KEY_AUDIO_SAMPLE_RATE, 8000);
        mSampleFormat = mediaInfo.getSampleFormat();
        openAudioTrack(mediaInfo);
    }

    @Override
    public void render() {

    }

    @Override
    public void release() {
        LogUtils.i(TAG, "CoreFlow : release");
        stopAudioTrack();
    }

    @Override
    public int write(@NonNull ByteBuffer outputBuffer, int size, long presentationTimeUs, int writeMode) {
        int result;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            mAudioTrack.setPlaybackParams(new PlaybackParams().setSpeed(
                    writeMode == AudioTrack.WRITE_BLOCKING ? 1.0f : 2.0f));
            result = mAudioTrack.write(outputBuffer, size, writeMode, presentationTimeUs);
        } else {
            result = mAudioTrack.write(outputBuffer, size, writeMode);
        }
        if (VERBOSE)
            LogUtils.d(TAG, "render audio " + presentationTimeUs + " " + size + " " + result);
        return result;
    }

    @Override
    public long getAudioTimeUs() {
        if (mAudioTrack == null) {
            return System.nanoTime() / 1000;
        }
        long systemClockUs = System.nanoTime() / 1000;
        if (systemClockUs - mLastTimestampSampleTimeUs >= MIN_TIMESTAMP_SAMPLE_INTERVAL_US) {
            mAudioTimestampSet = mAudioTrack.getTimestamp(mAudioTimestamp);
            if (!mAudioTimestampSet) {
                if (getLatencyMethod != null) {
                    try {
                        mLatencyUs = (Integer) getLatencyMethod.invoke(mAudioTrack, (Object[]) null) * 1000L / 2;
                        mLatencyUs = Math.max(mLatencyUs, 0);
                    } catch (Exception e) {
                        getLatencyMethod = null;
                    }
                }
            }
            mLastTimestampSampleTimeUs = systemClockUs;
        }

        long durationUs;
        if (mAudioTimestampSet) {
            // Calculate the speed-adjusted position using the timestamp (which may be in the future).
            long elapsedSinceTimestampUs = System.nanoTime() / 1000 - (mAudioTimestamp.nanoTime / 1000);
            long elapsedSinceTimestampFrames = elapsedSinceTimestampUs * mSampleRate / 1000000L;
            long elapsedFrames = mAudioTimestamp.framePosition + elapsedSinceTimestampFrames;
            durationUs = (elapsedFrames * 1000000L) / mSampleRate;
        } else {
            int numFramesPlayed = mAudioTrack.getPlaybackHeadPosition();
            durationUs = (numFramesPlayed * 1000000L) / mSampleRate - mLatencyUs;
        }
        return durationUs;
    }

    private void openAudioTrack(MediaInfo mediaInfo) {
        int sampleRate = mediaInfo.getInteger(MediaInfo.KEY_AUDIO_SAMPLE_RATE, 8000);
        int channelLayout = mediaInfo.getChannelLayout();
        int channels = mediaInfo.getInteger(MediaInfo.KEY_AUDIO_CHANNELS, 1);
        LogUtils.i(TAG, "openAudioTrack sampleRate = " + sampleRate + ", channelLayout = " +
                channelLayout + ", channels = " + channels + ", sampleFormat = " + mSampleFormat);
        try {
            int minBufferSize = AudioTrack.getMinBufferSize(sampleRate, channelLayout, mSampleFormat);
            mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate, channelLayout,
                    mSampleFormat, minBufferSize * 2, AudioTrack.MODE_STREAM);
            LogUtils.i(TAG, "openAudioTrack Audio Track min buffer size:" + minBufferSize);
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
