package com.gibbs.gplayer.codec;

import android.media.AudioFormat;
import android.media.AudioTrack;
import android.media.MediaCodec;
import android.media.MediaFormat;

import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.render.AudioRender;
import com.gibbs.gplayer.source.MediaSource;
import com.gibbs.gplayer.utils.LogUtils;

import java.nio.ByteBuffer;

public class MediaCodecAudioDecoder extends MediaCodecBaseDecoder implements AudioDecoder {
    private AudioRender mAudioRender;

    public MediaCodecAudioDecoder(MediaSource MediaSource) {
        mMediaSource = MediaSource;
    }

    @Override
    String getMineByAVHeader(MediaInfo header) {
        return C.getAudioMineByAVHeader(header);
    }

    void configure(String mine) {
        int sampleRate = mAVHeader.getInteger(MediaInfo.KEY_AUDIO_SAMPLE_RATE, 8000);
        int channelCount = mAVHeader.getInteger(MediaInfo.KEY_AUDIO_CHANNELS, 1);
        int channelLayout = mAVHeader.getChannelLayout();
        int profile = C.getAudioProfile(mAVHeader);

        MediaFormat format = MediaFormat.createAudioFormat(mine, sampleRate, channelCount);
        format.setInteger(MediaFormat.KEY_AAC_PROFILE, profile);
        format.setInteger(MediaFormat.KEY_PROFILE, profile);
        format.setInteger(MediaFormat.KEY_IS_ADTS, 1);
        format.setInteger(MediaFormat.KEY_CHANNEL_MASK, channelLayout);
        format.setByteBuffer("csd-0", C.getAacCsd0(mAVHeader));
        LogUtils.i(TAG, "input format = " + format.toString());
        codec.configure(format, null, null, 0);
        mMediaFormat = codec.getOutputFormat();
        LogUtils.i(TAG, "output format = " + mMediaFormat.toString());
        mAVHeader.setSampleFormat(AudioFormat.ENCODING_PCM_16BIT);
    }

    @Override
    MediaData readSource() {
        return mMediaSource.readAudioSource();
    }

    @Override
    void removeFirstPackage() {
        mMediaSource.removeFirstAudioPackage();
    }

    @Override
    void onOutputBuffer(int outputBufferId, ByteBuffer outputBuffer, MediaCodec.BufferInfo bufferInfo) {
        ByteBuffer audioBuffer = ByteBuffer.allocate(bufferInfo.size);
        audioBuffer.put(outputBuffer);
        audioBuffer.position(0);
        mOutputQueue.addLast(new MediaCodecOutputBuffer(audioBuffer, outputBufferId, bufferInfo));
        codec.releaseOutputBuffer(outputBufferId, false);
    }

    @Override
    public void renderBuffer() {
        synchronized (mOutputQueueLock) {
            MediaCodecOutputBuffer element = mOutputQueue.peekFirst();
            if (element != null) {
                if (mAudioRender != null) {
                    mAudioRender.write(element.outputBuffer, element.bufferInfo.size,
                            element.bufferInfo.presentationTimeUs, AudioTrack.WRITE_BLOCKING);
                }
                mOutputQueue.removeFirst();
            }
        }
    }

    @Override
    public void setAudioRender(AudioRender render) {
        mAudioRender = render;
    }
}
