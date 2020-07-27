package com.gibbs.gplayer.codec;

import android.media.MediaCodec;
import android.media.MediaFormat;
import android.text.TextUtils;

import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.source.MediaSource;
import com.gibbs.gplayer.utils.LogUtils;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.LinkedList;

public abstract class MediaCodecBaseDecoder implements BaseDecoder {
    String TAG = getClass().getSimpleName();

    private static final boolean VERBOSE = false;
    private static final long DEFAULT_TIMEOUT_US = 500;

    MediaCodec codec;
    MediaSource mMediaSource;
    MediaFormat mMediaFormat;
    LinkedList<MediaCodecOutputBuffer> mOutputQueue = new LinkedList<>();
    final Object mOutputQueueLock = new Object();
    MediaInfo mAVHeader;
    private Thread mDecodeThread;
    private boolean mIsDecoding;

    @Override
    public void init(MediaInfo header) {
        LogUtils.i(TAG, "init " + header.toString());
        codec = null;
        String mine = getMineByAVHeader(header);
        if (TextUtils.isEmpty(mine)) {
            LogUtils.e(TAG, "not support this media type");
            return;
        }
        try {
            codec = MediaCodec.createDecoderByType(mine);
        } catch (IOException e) {
            e.printStackTrace();
        }
        mAVHeader = header;
        configure(mine);
        start();
    }

    abstract String getMineByAVHeader(MediaInfo header);

    abstract void configure(String mine);

    private void start() {
        final boolean[] hasError = {false};
        mIsDecoding = true;
        mDecodeThread = new Thread(new Runnable() {
            @Override
            public void run() {
                LogUtils.i(TAG, "start decode");
                codec.start();
                while (mIsDecoding) {
                    try {
                        feedInputBuffer();
                        while (drainOutputBuffer()) {
                        }
                    } catch (IllegalStateException e) {
                        e.printStackTrace();
                        mIsDecoding = false;
                        hasError[0] = true;
                    }
                    if (mOutputQueue.size() > 2) {
                        try {
                            Thread.sleep(20);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                }
                if (codec != null) {
                    if (hasError[0]) {
                        codec.reset();
                        LogUtils.i(TAG, "reset decode");
                    } else {
                        codec.stop();
                        codec.release();
                        LogUtils.i(TAG, "stop decode");
                    }
                }
            }
        });
        mDecodeThread.start();
    }

    @Override
    public boolean feedInputBuffer() throws IllegalStateException {
        MediaData mediaData = readSource();
        if (mediaData == null) {
            return false;
        }
        int inputBufferId = codec.dequeueInputBuffer(DEFAULT_TIMEOUT_US);
        if (inputBufferId >= 0) {
            ByteBuffer inputBuffer = codec.getInputBuffer(inputBufferId);
            if (inputBuffer == null) {
                return false;
            }
            int flag = 0;
            if (mediaData.flag == 2) {
                flag |= MediaCodec.BUFFER_FLAG_CODEC_CONFIG;
                if (VERBOSE) LogUtils.d(TAG, Arrays.toString(mediaData.data.array()));
            } else if (mediaData.flag == 1) {
                flag |= MediaCodec.BUFFER_FLAG_KEY_FRAME;
            }
            inputBuffer.put(mediaData.data);
            if (VERBOSE)
                LogUtils.d(TAG, "feedInputBuffer success pts=" + mediaData.pts + ",size=" + mediaData.size + ",flag = " + flag);
            codec.queueInputBuffer(inputBufferId, 0, mediaData.size, mediaData.pts, flag);
            removeFirstPackage();
            return true;
        } else {
            if (VERBOSE) LogUtils.i(TAG, "feedInputBuffer error inputBufferId " + inputBufferId);
        }
        return false;
    }

    @Override
    public boolean drainOutputBuffer() throws IllegalStateException {
        MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
        int outputBufferId = codec.dequeueOutputBuffer(bufferInfo, DEFAULT_TIMEOUT_US);
        ByteBuffer outputBuffer = null;
        if (outputBufferId >= 0) {
            outputBuffer = codec.getOutputBuffer(outputBufferId);
        } else if (outputBufferId == MediaCodec.INFO_TRY_AGAIN_LATER) {
            if (VERBOSE) LogUtils.e(TAG, "drainOutputBuffer try later");
            return false;
        } else if (outputBufferId == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
            mMediaFormat = codec.getOutputFormat();
            LogUtils.e(TAG, "drainOutputBuffer format change " + mMediaFormat.toString());
        }
        if (outputBuffer == null) {
            return false;
        }
        onOutputBuffer(outputBufferId, outputBuffer, bufferInfo);
        if (VERBOSE)
            LogUtils.d(TAG, "outputBuffer mOutputQueue size=" + mOutputQueue.size() + ",pts=" + bufferInfo.presentationTimeUs);
        return true;
    }

    abstract void onOutputBuffer(int outputBufferId, ByteBuffer outputBuffer, MediaCodec.BufferInfo bufferInfo);

    @Override
    public void release() {
        LogUtils.i(TAG, "release");
        mIsDecoding = false;
        if (mDecodeThread != null && mDecodeThread.isAlive()) {
            try {
                mDecodeThread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        synchronized (mOutputQueueLock) {
            mOutputQueue.clear();
        }
    }

    abstract MediaData readSource();

    abstract void removeFirstPackage();
}
