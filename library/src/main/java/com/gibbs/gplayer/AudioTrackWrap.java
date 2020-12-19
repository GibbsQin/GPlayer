package com.gibbs.gplayer;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

import androidx.annotation.NonNull;

import java.nio.ByteBuffer;

public class AudioTrackWrap {
    private static final String TAG = "AudioTrackWrap";
    private AudioTrack mAudioTrack;

    public void openAudioTrack(int sampleRate, int sampleFormat, int channels, int bytesPerSample) {
        LogUtils.i(TAG, "openAudioTrack " + sampleRate + " " + sampleFormat + " " + channels + " " + bytesPerSample);
        try {
            sampleFormat = getSampleFormat(sampleFormat);
            int channelLayout = (int) getChannelLayout(channels);
            int minBufferSize = AudioTrack.getMinBufferSize(sampleRate, channelLayout, sampleFormat);
            int bufferSizeInBytes = Math.round(minBufferSize * 1.0f / bytesPerSample + 0.5f) * bytesPerSample;
            mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate, channelLayout,
                    sampleFormat, minBufferSize * 2, AudioTrack.MODE_STREAM);
            mAudioTrack.play();
            LogUtils.i(TAG, "openAudioTrack min buffer size:" + minBufferSize + ", bufferSizeInBytes:" + bufferSizeInBytes);
        } catch (Exception e) {
            LogUtils.e(TAG, "create AudioTrack error: " + e.getMessage());
        }
    }

    public int write(@NonNull ByteBuffer buffer, int size) {
        int sizeWrote = mAudioTrack.write(buffer, size, AudioTrack.WRITE_BLOCKING);
        LogUtils.i(TAG, "write buffer size " + size + ", sizeWrote " + sizeWrote);
        return sizeWrote;
    }

    public void stopAudioTrack() {
        if (mAudioTrack != null) {
            mAudioTrack.flush();
            mAudioTrack.stop();
            mAudioTrack.release();
            mAudioTrack = null;
            LogUtils.i(TAG, "stopAudioTrack");
        }
    }

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

    private int getSampleFormat(int sfmt) {
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

    private long getChannelLayout(int channels) {
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
}
