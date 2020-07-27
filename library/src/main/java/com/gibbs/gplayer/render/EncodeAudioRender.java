package com.gibbs.gplayer.render;

import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.codec.AudioDecoder;
import com.gibbs.gplayer.codec.MediaCodecAudioDecoder;
import com.gibbs.gplayer.source.MediaSource;

public class EncodeAudioRender extends BaseAudioRender {
    private AudioDecoder mAudioDecoder;

    public EncodeAudioRender(MediaSource source) {
        super(source);
        mAudioDecoder = new MediaCodecAudioDecoder(mMediaSource);
        mAudioDecoder.setAudioRender(this);
    }

    @Override
    public void init(MediaInfo mediaInfo) {
        mAudioDecoder.init(mediaInfo);
        super.init(mediaInfo);
    }

    @Override
    public void render() {
        super.render();
        mAudioDecoder.renderBuffer();
    }

    @Override
    public void release() {
        super.release();
        mAudioDecoder.release();
    }
}
