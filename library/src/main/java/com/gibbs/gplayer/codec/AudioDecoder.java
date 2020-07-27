package com.gibbs.gplayer.codec;

import com.gibbs.gplayer.render.AudioRender;

public interface AudioDecoder extends BaseDecoder {
    void setAudioRender(AudioRender render);
}
