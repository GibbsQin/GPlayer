package com.gibbs.gplayer.source;

import android.media.AudioFormat;
import android.media.AudioTrack;

import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.utils.LogUtils;

public class MediaSourceImp implements MediaSource {
    private static final String TAG = "MediaSourceImp";

    /**
     * ffmpeg sample fmt
     */
    private static final int FFMPEG_SAMPLE_FMT_U8 = 0;          ///< unsigned 8 bits
    private static final int FFMPEG_SAMPLE_FMT_S16 = 1;         ///< signed 16 bits
    private static final int FFMPEG_SAMPLE_FMT_S32 = 2;         ///< signed 32 bits
    private static final int FFMPEG_SAMPLE_FMT_FLT = 3;         ///< float
    private static final int FFMPEG_SAMPLE_FMT_DBL = 4;         ///< double
    private static final int FFMPEG_SAMPLE_FMT_U8P = 5;         ///< unsigned 8 bits, planar
    private static final int FFMPEG_SAMPLE_FMT_S16P = 6;        ///< signed 16 bits, planar
    private static final int FFMPEG_SAMPLE_FMT_S32P = 7;        ///< signed 32 bits, planar
    private static final int FFMPEG_SAMPLE_FMT_FLTP = 8;        ///< float, planar
    private static final int FFMPEG_SAMPLE_FMT_DBLP = 9;        ///< double, planar
    private static final int FFMPEG_SAMPLE_FMT_S64 = 10;        ///< signed 64 bits
    private static final int FFMPEG_SAMPLE_FMT_S64P = 11;       ///< signed 64 bits, planar

    private static final int VSYNC_DURATION_MS = 32;
    private static final int MAX_SLEEP_DURATION_MS = 50;

    private static class AudioSink {
        int bufferSizeInBytes;
        int durationMs;
        long pendingRenderTimeMs;
        long queueTimeNs;

        AudioSink(int sampleRate, int channelLayout, int sampleFormat, int bytesPerFrame) {
            int minBufferSize = AudioTrack.getMinBufferSize(sampleRate, channelLayout, sampleFormat);
            bufferSizeInBytes = Math.round(minBufferSize * 1.0f / bytesPerFrame + 0.5f) * bytesPerFrame;
            durationMs = (int) (bufferSizeInBytes * 1.0f / (sampleRate * getBytesPerSample(sampleFormat)) * 1000);
            LogUtils.i(TAG, "durationMs = " + durationMs);
        }

        int getBytesPerSample(int audioFormat) {
            switch (audioFormat) {
                case AudioFormat.ENCODING_PCM_8BIT:
                    return 1;
                case AudioFormat.ENCODING_PCM_16BIT:
                case AudioFormat.ENCODING_IEC61937:
                case AudioFormat.ENCODING_DEFAULT:
                    return 2;
                case AudioFormat.ENCODING_PCM_FLOAT:
                    return 4;
                case AudioFormat.ENCODING_INVALID:
                default:
                    throw new IllegalArgumentException("Bad audio format " + audioFormat);
            }
        }

        void queueSample(long pts) {
            pendingRenderTimeMs = pts;
            queueTimeNs = System.nanoTime();
        }

        long getNow() {
            long currentNs = System.nanoTime();
            long passTimeMs = (currentNs - queueTimeNs) / 1000_000;
            long startTimeMs = pendingRenderTimeMs - durationMs / 2;
            return startTimeMs + passTimeMs;
        }
    }

    private int mChannelId;
    private MediaData mTopAudioFrame = null;
    private MediaData mTopVideoFrame = null;
    private AudioSink mAudioSink;

    public MediaSourceImp(int channel) {
        mChannelId = channel;
    }

    @Override
    public void init() {
        int sampleRate = getSampleRate();
        int channelLayout = (int) getChannelLayout();
        int format = getSampleFormat();
        int bytesPerSample = getBytesPerFrame();
        LogUtils.i(TAG, "sampleRate = " + sampleRate);
        LogUtils.i(TAG, "channelLayout = " + channelLayout);
        LogUtils.i(TAG, "format = " + format);
        LogUtils.i(TAG, "bytesPerSample = " + bytesPerSample);
        mAudioSink = new AudioSink(sampleRate, channelLayout, format, bytesPerSample);
    }

    @Override
    public MediaData readAudioSource() {
        if (mTopAudioFrame == null) {
            mTopAudioFrame = readAudioSource(mChannelId);
        }
        return mTopAudioFrame;
    }

    @Override
    public MediaData readVideoSource() {
        if (mTopVideoFrame == null) {
            mTopVideoFrame = readVideoSource(mChannelId);
        }
        if (mTopVideoFrame == null) {
            try {
                Thread.sleep(20);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            return null;
        }
        long nowMs = mAudioSink.getNow();
        long lateMs = mTopVideoFrame.pts - (nowMs - VSYNC_DURATION_MS);
        if (lateMs < 0) {//视频落后，开始播放
            return mTopVideoFrame;
        } else if (lateMs < MAX_SLEEP_DURATION_MS) {
            try {
                Thread.sleep(lateMs);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            return mTopVideoFrame;
        } else {
            try {
                Thread.sleep(MAX_SLEEP_DURATION_MS);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            return null;
        }
    }

    @Override
    public void removeFirstAudioPackage() {
        mAudioSink.queueSample(mTopAudioFrame.pts);
        mTopAudioFrame = null;
        removeFirstAudioPackage(mChannelId);
    }

    @Override
    public void removeFirstVideoPackage() {
        mTopVideoFrame = null;
        removeFirstVideoPackage(mChannelId);
    }

    @Override
    public void flushBuffer() {
        flushBuffer(mChannelId);
        mAudioSink = null;
        mTopVideoFrame = null;
        mTopAudioFrame = null;
    }

    @Override
    public int getAudioBufferSize() {
        return getAudioBufferSize(mChannelId);
    }

    @Override
    public int getVideoBufferSize() {
        return getVideoBufferSize(mChannelId);
    }

    @Override
    public int getFrameRate() {
        return getFrameRate(mChannelId);
    }

    @Override
    public int getDuration() {
        return getDuration(mChannelId);
    }

    @Override
    public int getSampleRate() {
        return getSampleRate(mChannelId);
    }

    @Override
    public int getSampleFormat() {
        int sfmt = getSampleFormat(mChannelId);
        int sampleFormat = AudioFormat.ENCODING_PCM_16BIT;
        switch (sfmt) {
            case FFMPEG_SAMPLE_FMT_U8:
                sampleFormat = AudioFormat.ENCODING_PCM_8BIT;
                break;
            case FFMPEG_SAMPLE_FMT_S16:
                sampleFormat = AudioFormat.ENCODING_PCM_16BIT;
                break;
            case FFMPEG_SAMPLE_FMT_FLTP:
                sampleFormat = AudioFormat.ENCODING_PCM_FLOAT;
                break;
        }

        return sampleFormat;
    }

    @Override
    public long getChannelLayout() {
        int channels = getChannels(mChannelId);
        int chanelLayout;
        switch (channels) {
            case 1:
                chanelLayout = AudioFormat.CHANNEL_OUT_MONO;
                break;
            case 2:
                chanelLayout = AudioFormat.CHANNEL_OUT_STEREO;
                break;
            case 3:
                chanelLayout = AudioFormat.CHANNEL_OUT_SURROUND;
                break;
            case 4:
                chanelLayout = AudioFormat.CHANNEL_OUT_QUAD;
                break;
            case 6:
                chanelLayout = AudioFormat.CHANNEL_OUT_5POINT1;
                break;
            default:
                chanelLayout = AudioFormat.CHANNEL_OUT_DEFAULT;
        }
        return chanelLayout;
    }

    @Override
    public int getChannels() {
        return getChannels(mChannelId);
    }

    @Override
    public int getWidth() {
        return getWidth(mChannelId);
    }

    @Override
    public int getHeight() {
        return getHeight(mChannelId);
    }

    @Override
    public int getRotate() {
        return getRotate(mChannelId);
    }

    @Override
    public int getBytesPerFrame() {
        return getBytesPerFrame(mChannelId);
    }

    private native MediaData readAudioSource(int channelId);

    private native MediaData readVideoSource(int channelId);

    private native void removeFirstAudioPackage(int channelId);

    private native void removeFirstVideoPackage(int channelId);

    private native void flushBuffer(int channelId);

    private native int getAudioBufferSize(int channelId);

    private native int getVideoBufferSize(int channelId);

    private native int getFrameRate(int channelId);

    private native int getDuration(int channelId);

    private native int getSampleRate(int channelId);

    private native int getSampleFormat(int channelId);

    private native int getChannels(int channelId);

    private native int getWidth(int channelId);

    private native int getHeight(int channelId);

    private native int getRotate(int channelId);

    private native int getBytesPerFrame(int channelId);
}
