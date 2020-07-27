package com.gibbs.gplayer.source;

import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.media.MediaInfo;

public interface MediaSource extends MediaSourceControl {
    /**
     * the url is a local file
     */
    int SOURCE_TYPE_FILE = 0;
    /**
     * not support now
     */
    int SOURCE_TYPE_P2P = 1;

    /**
     * decode the media data on jni layer
     */
    int FLAG_DECODE = 0x00000001;
    /**
     * decode the media data using mediacodec
     */
    int FLAG_MEDIA_CODEC = 0x00000002;

    void onInit(int channelId, MediaInfo header);

    int onReceiveAudio(MediaData inPacket);

    int onReceiveVideo(MediaData inPacket);

    void onRelease();

    void onError(int errorCode, String errorMessage);

    void setOnSourceStateChangedListener(OnSourceStateChangedListener listener);

    MediaData readAudioSource();

    MediaData readVideoSource();

    void removeFirstAudioPackage();

    void removeFirstVideoPackage();

    void clearAudioQueue();

    void clearVideoQueue();

    void flushBuffer();

    void checkSourceEnd();

    void onRemoteAudioSizeChanged(int size);

    void onRemoteVideoSizeChanged(int size);
}
