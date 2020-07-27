package com.gibbs.gplayer.render;

import android.media.AudioTrack;

import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.source.MediaSource;

public class PcmAudioRender extends BaseAudioRender {
    private static final String TAG = "PcmAudioRender";

    public PcmAudioRender(MediaSource source) {
        super(source);
    }

    @Override
    public void render() {
        super.render();
        MediaData mediaData = mMediaSource.readAudioSource();
        if (mediaData != null) {
            write(mediaData.data, mediaData.size, mediaData.pts, AudioTrack.WRITE_BLOCKING);
            mMediaSource.removeFirstAudioPackage();
        }
    }
}
