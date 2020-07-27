package com.gibbs.gplayer.codec;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;

import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.utils.LogUtils;

import java.io.IOException;
import java.nio.ByteBuffer;

public class MediaCodecAudioEncoder implements AudioEncoder {
    private static final String TAG = "MediaCodecAudioEncoder";

    private static final boolean VERBOSE = false;

    private static final int KEY_SAMPLE_RATE = 44100;
    private static final int KEY_BIT_RATE = 64000;
    private static final int KEY_AAC_PROFILE = MediaCodecInfo.CodecProfileLevel.AACObjectLC;
    private static final boolean NEED_ADD_ADTS = false;

    private boolean mNeedAddADTS = NEED_ADD_ADTS;
    private int mFrameSize = 2048;
    private int mSampleRate = 8000;
    private MediaCodec mEncoder;
    private MediaCodec.BufferInfo mBufferInfo;
    private MediaFormat mMediaFormat;
    private float mFrameRate;
    private long mPts;

    public MediaCodecAudioEncoder() {
        this(true);
    }

    public MediaCodecAudioEncoder(boolean needAddADTA) {
        mNeedAddADTS = needAddADTA;
    }

    @Override
    public void init(MediaInfo header) {
        LogUtils.i(TAG, "init " + header.toString());
        if (mEncoder != null) {
            LogUtils.e(TAG, "encoder has been created");
            return;
        }
        prepareEncode(header);
        mEncoder.start();
        mFrameSize = header.getInteger(MediaInfo.KEY_AUDIO_SAMPLE_NUM_PERFRAME, mFrameSize);
        mSampleRate = header.getInteger(MediaInfo.KEY_AUDIO_SAMPLE_RATE, KEY_SAMPLE_RATE);
        mFrameRate = mSampleRate * 1.0f / mFrameSize;
        mPts = 0;
    }

    @Override
    public int send_frame(MediaData inFrame) {
        int inputBufferIndex = mEncoder.dequeueInputBuffer(-1);
        if (inputBufferIndex >= 0) {
            ByteBuffer inputBuffer = mEncoder.getInputBuffer(inputBufferIndex);
            if (inputBuffer == null) {
                return C.MEDIA_CODEC_INVALID_BUFFER;
            }
            inputBuffer.clear();
            inputBuffer.put(inFrame.data);
            inputBuffer.position(0);
            inputBuffer.limit(inFrame.size);
            mEncoder.queueInputBuffer(inputBufferIndex, 0, inFrame.size, mPts, 0);
            if (VERBOSE) LogUtils.d(TAG, "send_frame pts = " + mPts);
            mPts += 1_000_000 / mFrameRate;
            return C.MEDIA_CODEC_SUCCESS;
        }
        return C.MEDIA_CODEC_INVALID_BUFFER_INDEX;
    }

    @Override
    public int receive_packet(MediaData outPacket) {
        return receive_packet(outPacket, mBufferInfo);
    }

    public int receive_packet(MediaData outPacket, MediaCodec.BufferInfo bufferInfo) {
        int outputBufferIndex = mEncoder.dequeueOutputBuffer(bufferInfo, 0);
        if (outputBufferIndex >= 0) {
            ByteBuffer outputBuffer = mEncoder.getOutputBuffer(outputBufferIndex);
            if (outputBuffer == null) {
                return C.MEDIA_CODEC_INVALID_BUFFER;
            }

            if (mNeedAddADTS) {
                int length = bufferInfo.size + 7;
                outPacket.data.position(0);
                //给adts头字段空出7的字节
                addADTStoPacket(outPacket.data, length);
            } else {
                outPacket.data.position(0);
            }
            outPacket.data.put(outputBuffer);
            outPacket.data.position(0);
            outPacket.size = outPacket.data.remaining();
            outPacket.pts = bufferInfo.presentationTimeUs;
            mEncoder.releaseOutputBuffer(outputBufferIndex, false);
            if (VERBOSE) LogUtils.d(TAG, "receive_packet pts = " + outPacket.pts);
            return C.MEDIA_CODEC_SUCCESS;
        } else if (outputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
            mMediaFormat = mEncoder.getOutputFormat();
            LogUtils.e(TAG, "drainOutputBuffer format change " + mMediaFormat.toString());
            return C.MEDIA_CODEC_FORMAT_CHANGED;
        }
        return C.MEDIA_CODEC_INVALID_BUFFER_INDEX;
    }

    @Override
    public void release() {
        if (mEncoder != null) {
            mEncoder.stop();
            mEncoder.release();
            mEncoder = null;
        }
    }

    private void prepareEncode(MediaInfo header) {
        try {
            mBufferInfo = new MediaCodec.BufferInfo();
            String mimeType = C.getAudioMineByAVHeader(header);
            int sampleRate = header.getInteger(MediaInfo.KEY_AUDIO_SAMPLE_RATE, KEY_SAMPLE_RATE);
            int channelCount = header.getInteger(MediaInfo.KEY_AUDIO_CHANNELS, 1);
            int bitRate = header.getInteger(MediaInfo.KEY_BIT_RATE, KEY_BIT_RATE);
            mEncoder = MediaCodec.createEncoderByType(mimeType);
            MediaFormat mediaFormat = MediaFormat.createAudioFormat(mimeType, sampleRate, channelCount);
            mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, bitRate);
            mediaFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, KEY_AAC_PROFILE);
            mEncoder.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * 给编码出的aac裸流添加adts头字段
     *
     * @param packet    要空出前7个字节，否则会搞乱数据
     * @param packetLen aac es的长度
     */
    private void addADTStoPacket(ByteBuffer packet, int packetLen) {
        int profile = 2;  //AAC LC
        int freqIdx = C.sample2MediaCodecIndex(mSampleRate);
        int chanCfg = 2;  //CPE
        packet.put((byte) 0xFF);
        packet.put((byte) 0xF9);
        packet.put((byte) (((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2)));
        packet.put((byte) (((chanCfg & 3) << 6) + (packetLen >> 11)));
        packet.put((byte) ((packetLen & 0x7FF) >> 3));
        packet.put((byte) (((packetLen & 7) << 5) + 0x1F));
        packet.put((byte) 0xFC);
    }

    public MediaFormat getMediaFormat() {
        return mMediaFormat;
    }
}
