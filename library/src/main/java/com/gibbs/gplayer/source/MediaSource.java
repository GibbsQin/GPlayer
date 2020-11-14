package com.gibbs.gplayer.source;

import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.media.MediaInfo;

public interface MediaSource extends MediaSourceControl {
    /**
     * decode the media data on jni layer
     */
    int FLAG_DECODE = 0x00000001;
    /**
     * decode the media data using mediacodec
     */
    int FLAG_MEDIA_CODEC = 0x00000002;

    /**
     * Init : has received media head info
     * Ready : playing
     * Release : media source was release
     * Error : error occur
     * Error : media source connection is disconnecting
     */
    enum SourceState {
        Init, Ready, Release, Error, Finishing
    }

    void onInit(int channelId, MediaInfo header);

    int onReceiveAudio(MediaData inPacket);

    int onReceiveVideo(MediaData inPacket);

    void onRelease();

    void onError(int errorCode, String errorMessage);

    void onFinishing();

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
