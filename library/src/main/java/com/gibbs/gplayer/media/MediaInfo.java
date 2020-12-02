package com.gibbs.gplayer.media;

import android.media.AudioFormat;

import java.util.HashMap;
import java.util.Map;

public class MediaInfo {
    /**
     * 文件长度(毫秒)
     * The associated value is an integer
     */
    public static final String KEY_DURATION = "duration";

    /**
     * 视频编码类型 h264 h265
     * The associated value is an integer
     */
    public static final String KEY_VIDEO_TYPE = "video-type";

    /**
     * A key describing the width of the content in a video format.
     * The associated value is an integer
     */
    public static final String KEY_WIDTH = "width";

    /**
     * A key describing the height of the content in a video format.
     * The associated value is an integer
     */
    public static final String KEY_HEIGHT = "height";

    /**
     * frame rate
     * The associated value is an integer
     */
    public static final String KEY_FRAME_RATE = "frame-rate";

    /**
     * A key describing the average bitrate in bits/sec.
     * The associated value is an integer
     */
    public static final String KEY_BIT_RATE = "bitrate";


    //音频编码格式
    public static final String KEY_AUDIO_TYPE = "audio-type";

    //音频编码的参数
    public static final String KEY_AUDIO_CODEC_OPTION = "audio-codec-option";

    // 音频模式： 单声道/双声道
    private static final String KEY_AUDIO_MODE = "audio-mode";

    // Codec-specific bitstream restrictions that the stream conforms to.
    public static final String KEY_AUDIO_PROFILE = "audio-profile";

    // 声道数
    public static final String KEY_AUDIO_CHANNELS = "audio-channels";

    // 音频位宽
    private static final String KEY_AUDIO_BIT_WIDTH = "audio-bit-width";

    // 音频采样率
    public static final String KEY_AUDIO_SAMPLE_RATE = "audio-sample-rate";

    //每帧数据里的采样数
    public static final String KEY_AUDIO_SAMPLE_NUM_PERFRAME = "audio-sample-num-perframe";

    // 视频旋转角度
    public static final String KEY_VIDEO_ROTATE = "video-rotate";


    //数据字典
    private Map<String, Object> map;

    public MediaInfo() {
        map = new HashMap<>();
    }

    /**
     * Returns true iff a key of the given name exists in the format.
     */
    public final boolean containsKey(String name) {
        return map.containsKey(name);
    }

    /**
     * Returns the value of a string key.
     *
     * @return null if the key does not exist or the stored value for the key is null
     * @throws ClassCastException if the stored value for the key is int, long, float or ByteBuffer
     */
    public final String getString(String name) {
        return (String) map.get(name);
    }

    /**
     * Returns the value of an string key, or the default value if the key is missing.
     *
     * @return defaultValue if the key does not exist or the stored value for the key is null
     * @throws ClassCastException if the stored value for the key is int, long, float or ByteBuffer
     */
    public final String getString(String name, String defaultValue) {
        String ret = getString(name);
        return ret == null ? defaultValue : ret;
    }

    /**
     * Returns the value of an integer key.
     *
     * @throws NullPointerException if the key does not exist or the stored value for the key is
     *                              null
     * @throws ClassCastException   if the stored value for the key is long, float, ByteBuffer or
     *                              String
     */
    private final int getInteger(String name) {
        return ((Integer) map.get(name)).intValue();
    }

    /**
     * Returns the value of an integer key, or the default value if the key is missing.
     *
     * @return defaultValue if the key does not exist or the stored value for the key is null
     * @throws ClassCastException if the stored value for the key is long, float, ByteBuffer or
     *                            String
     */
    public final int getInteger(String name, int defaultValue) {
        try {
            return getInteger(name);
        } catch (NullPointerException e) {
            /* no such field or field is null */
            return defaultValue;
        }
    }

    public final void setInteger(String name, int value) {
        map.put(name, value);
    }

    @Override
    public String toString() {
        return "MediaHeader{" +
                "map=" + map +
                '}';
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

    public int getChannelLayout() {
        int chanelLayout;
        int channels = getInteger(KEY_AUDIO_CHANNELS, 1);
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

    public void setChannelLayout(int channelLayout) {
        if (channelLayout == AudioFormat.CHANNEL_OUT_MONO) {
            setInteger(KEY_AUDIO_CHANNELS, 1);
        } else {
            setInteger(KEY_AUDIO_CHANNELS, 2);
        }
    }

    public int getSampleFormat() {
        int sampleFormat = AudioFormat.ENCODING_PCM_16BIT;
        switch (getInteger(KEY_AUDIO_BIT_WIDTH, FFMPEG_SAMPLE_FMT_S16)) {
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

    public void setSampleFormat(int sampleFormat) {
        if (sampleFormat == AudioFormat.ENCODING_PCM_8BIT) {
            setInteger(KEY_AUDIO_BIT_WIDTH, FFMPEG_SAMPLE_FMT_U8);
        } else {
            setInteger(KEY_AUDIO_BIT_WIDTH, FFMPEG_SAMPLE_FMT_S16);
        }
    }
}
