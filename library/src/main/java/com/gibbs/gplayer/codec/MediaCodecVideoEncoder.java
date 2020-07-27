package com.gibbs.gplayer.codec;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;

import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.utils.LogUtils;

import java.io.IOException;
import java.nio.ByteBuffer;

public class MediaCodecVideoEncoder implements VideoEncoder {
    private static final String TAG = "MediaCodecVideoEncoder";

    private static final boolean VERBOSE = false;

    private MediaCodec mEncoder;
    private MediaCodec.BufferInfo mBufferInfo = new MediaCodec.BufferInfo();
    private MediaFormat mMediaFormat;
    private int mColorFormat = MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible;
    private int mBitRate = 200000;
    private float mFrameRate;
    private long mPts;

    public MediaCodecVideoEncoder() {

    }

    public MediaCodecVideoEncoder(int colorFormat) {
        mColorFormat = colorFormat;
    }

    public MediaCodecVideoEncoder(int colorFormat, int bitRate) {
        mColorFormat = colorFormat;
        mBitRate = bitRate;
    }

    @Override
    public void init(MediaInfo header) {
        mPts = 0;
        prepareEncode(header);
        mEncoder.start();
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
            inputBuffer.position(0);
            inputBuffer.put(formatYUV420Data(inFrame));
            inputBuffer.position(0);
            int size = inFrame.size + inFrame.size1 + inFrame.size2;
            inputBuffer.limit(size);
            mEncoder.queueInputBuffer(inputBufferIndex, 0, size, mPts, 0);
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

            outPacket.data.position(0);
            outPacket.data.put(outputBuffer);
            outPacket.data.position(0);
            outPacket.size = bufferInfo.size;
            outPacket.pts = bufferInfo.presentationTimeUs;
            outPacket.flag = (bufferInfo.flags | MediaCodec.BUFFER_FLAG_KEY_FRAME);
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

    private ByteBuffer formatYUV420Data(MediaData mediaData) {
        ByteBuffer byteBuffer = ByteBuffer.allocate(mediaData.size + mediaData.size1 + mediaData.size2);
        byteBuffer.put(mediaData.data);
        for (int j = 0; j < mediaData.size2; j++) {
            byteBuffer.put(mediaData.data1.get(j));
            byteBuffer.put(mediaData.data2.get(j));
        }
        byteBuffer.position(0);
        return byteBuffer;
    }

    @Override
    public void release() {
        stopEncode();
    }

    private void prepareEncode(MediaInfo header) {
        LogUtils.i(TAG, "prepareEncode");
        mBufferInfo = new MediaCodec.BufferInfo();
        try {
            mEncoder = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_VIDEO_AVC);
        } catch (IOException e) {
            e.printStackTrace();
        }
        int width = header.getInteger(MediaInfo.KEY_WIDTH, 0);
        int height = header.getInteger(MediaInfo.KEY_HEIGHT, 0);
        MediaFormat mediaFormat = MediaFormat.createVideoFormat(MediaFormat.MIMETYPE_VIDEO_AVC, width, height);
        mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, mBitRate);
        mFrameRate = header.getInteger(MediaInfo.KEY_FRAME_RATE, 20);
        mediaFormat.setFloat(MediaFormat.KEY_FRAME_RATE, mFrameRate);
        mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, mColorFormat);
        mediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 40);
        mEncoder.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
    }

    private void stopEncode() {
        if (mEncoder == null) {
            return;
        }
        LogUtils.i(TAG, "stopEncode");
        try {
            mEncoder.stop();
            mEncoder.release();
            mEncoder = null;
        } catch (Exception e) {
            e.printStackTrace();
            mEncoder = null;
        }
    }

    public MediaFormat getMediaFormat() {
        return mMediaFormat;
    }
}
