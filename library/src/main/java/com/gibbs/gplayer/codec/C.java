package com.gibbs.gplayer.codec;

import android.media.MediaCodecInfo;
import android.media.MediaFormat;

import com.gibbs.gplayer.media.MediaConstant;
import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.utils.ByteUtils;
import com.gibbs.gplayer.utils.LogUtils;

import java.nio.ByteBuffer;

public class C {
    private static final String TAG = "C";

    private static final int NAL_UNIT_TYPE_NON_IDR = 1; // Coded slice of a non-IDR picture
    private static final int NAL_UNIT_TYPE_PARTITION_A = 2; // Coded slice data partition A
    private static final int NAL_UNIT_TYPE_IDR = 5; // Coded slice of an IDR picture
    private static final int NAL_UNIT_TYPE_SPS = 7; // Coded slice of an SPS
    private static final int NAL_UNIT_TYPE_AUD = 9; // Access unit delimiter

    public static final int MEDIA_CODEC_SUCCESS = 0;
    public static final int MEDIA_CODEC_INVALID_BUFFER = -1;
    public static final int MEDIA_CODEC_INVALID_BUFFER_INDEX = -2;
    public static final int MEDIA_CODEC_FORMAT_CHANGED = 1;

    private static final int[] SAMPLE_RATE_MAP = {
        96000, 0x00,
        88200, 0x01,
        64000, 0x02,
        48000, 0x03,
        44100, 0x04,
        32000, 0x05,
        24000, 0x06,
        22050, 0x07,
        16000, 0x08,
        12000, 0x09,
        11025, 0x0A,
        8000,  0x0B
    };


    static String getVideoMineByAVHeader(MediaInfo mediaInfo) throws IllegalArgumentException {
        int videoType = mediaInfo.getInteger(MediaInfo.KEY_VIDEO_TYPE, MediaConstant.VIDEO_TYPE_H264);
        if (videoType == MediaConstant.VIDEO_TYPE_H264) {
            return MediaFormat.MIMETYPE_VIDEO_AVC;
        } else if (videoType == MediaConstant.VIDEO_TYPE_H265) {
            return MediaFormat.MIMETYPE_VIDEO_HEVC;
        }

        return null;
    }

    static String getAudioMineByAVHeader(MediaInfo mediaInfo) throws IllegalArgumentException {
        int audioType = mediaInfo.getInteger(MediaInfo.KEY_AUDIO_TYPE, MediaConstant.AUDIO_TYPE_AAC);
        if (audioType == MediaConstant.AUDIO_TYPE_AAC) {
            return MediaFormat.MIMETYPE_AUDIO_AAC;
        }

        return null;
    }

    static int getAudioProfile(MediaInfo mediaInfo) {
        int profile = mediaInfo.getInteger(MediaInfo.KEY_AUDIO_PROFILE, 1);//AAC Main 0x01, LC 0x02, SSR 0x03
        if (profile == 1) {
            return MediaCodecInfo.CodecProfileLevel.AACObjectLC;
        }
        return MediaCodecInfo.CodecProfileLevel.AACObjectLC;
    }

    static ByteBuffer getAacCsd0(MediaInfo mediaInfo) {
        int sampleRate = mediaInfo.getInteger(MediaInfo.KEY_AUDIO_SAMPLE_RATE, 8000);
        int channelCount = mediaInfo.getInteger(MediaInfo.KEY_AUDIO_CHANNELS, 1);
        int profile = getAudioProfile(mediaInfo);

        sampleRate = sample2MediaCodecIndex(sampleRate);
        LogUtils.i(TAG, "getAacCsd0 sampleRate = " + sampleRate + ", channelCount = " + channelCount + ", profile = " + profile);

        short csd_0 = 0;
        csd_0 |= (profile << 11);
        csd_0 |= (sampleRate << 7);
        csd_0 |= (channelCount << 3);

        ByteBuffer csdByteBuffer = ByteBuffer.allocate(2).put(new byte[]{(byte) (csd_0 >> 8 & 0xFF), (byte) (csd_0 & 0xFF)});
        csdByteBuffer.position(0);

        return csdByteBuffer;
    }

    static int sample2MediaCodecIndex(int sampleRate) {
        for (int i = 0; i < SAMPLE_RATE_MAP.length; i += 2) {
            if (SAMPLE_RATE_MAP[i] == sampleRate) {
                sampleRate = SAMPLE_RATE_MAP[i+1];
                break;
            }
        }
        return sampleRate;
    }

    static boolean isH264KeyFrame(byte[] data) {
        int nalHead = ByteUtils.bytesToInt(data, 0);
        int nalType = data[4] & 0x1F;
        return nalHead == 0x01000000 &&
                (nalType == NAL_UNIT_TYPE_IDR || nalType == NAL_UNIT_TYPE_SPS);
    }

    static int getNalUnitType(byte[] data) {
        int nalType;
        nalType = data[4] & 0x1F;
        return nalType;
    }

    static String logAVData(MediaData mediaData) {
        return "width:" + mediaData.width + " height:" + mediaData.height + " pts:" + mediaData.pts +
                " size: " + mediaData.size + " " + mediaData.size1 + " " + mediaData.size2;
    }
}
