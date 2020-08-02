package com.gibbs.gplayer.source;

import android.text.TextUtils;

import androidx.annotation.NonNull;

import com.gibbs.gplayer.media.MediaData;
import com.gibbs.gplayer.media.MediaInfo;
import com.gibbs.gplayer.utils.LogUtils;

import java.util.LinkedList;

public class MediaSourceImp implements MediaSource {
    private static final String TAG = "MediaSourceImpJ";

    private static final boolean VERBOSE = false;

    private int mChannelId;
    private String mUrl;
    private MediaInfo mMediaInfo;
    private int mFlag = 0;
    private LinkedList<MediaData> mAudioQueue;
    private LinkedList<MediaData> mVideoQueue;
    private final Object mAudioQueueLock = new Object();
    private final Object mVideoQueueLock = new Object();
    private boolean mIsSourceEnd;

    private SourceState mSourceState = null;
    private OnSourceStateChangedListener mOnSourceStateChangedListener;
    private OnSourceSizeChangedListener mOnSourceSizeChangedListener;
    private OnTimeChangedListener mOnTimeChangedListener;
    private OnErrorListener mOnErrorListener;

    public MediaSourceImp(int type, String url) {
        this(type, url, false);
    }

    public MediaSourceImp(int type, String url, boolean decode) {
        this(type, url, decode, false);
    }

    public MediaSourceImp(int type, String url, boolean decode, boolean mediaCodec) {
        setUrl(type, url);
        mAudioQueue = new LinkedList<>();
        mVideoQueue = new LinkedList<>();
        if (decode) {
            mFlag |= FLAG_DECODE;
        }
        if (mediaCodec) {
            mFlag |= FLAG_MEDIA_CODEC;
        }
        LogUtils.i(TAG, "CoreFlow : new MediaSourceImp url = " + url + ", decode = " + decode + ", mediaCodec = "
                + mediaCodec + ", flag = " + mFlag);
    }

    @Override
    public void onInit(int channelId, MediaInfo header) {
        LogUtils.i(TAG, "CoreFlow : onInit channelId = " + channelId+ ", header = " + header.toString());
        mChannelId = channelId;
        mMediaInfo = header;
        setSourceState(SourceState.Init);
        mIsSourceEnd = false;
    }

    @Override
    public int onReceiveAudio(MediaData inPacket) {
        if (mSourceState == SourceState.Init) {
            setSourceState(SourceState.Ready);
        }
        synchronized (mAudioQueueLock) {
            MediaData dstData = new MediaData(inPacket);
            long presentUs = inPacket.pts;
            dstData.pts = presentUs;
            mAudioQueue.add(dstData);
            if (VERBOSE) LogUtils.i(TAG, "receive new audio package " + presentUs + ", size = " + mAudioQueue.size());
        }
        if (mOnSourceSizeChangedListener != null) {
            mOnSourceSizeChangedListener.onLocalAudioSizeChanged(mAudioQueue.size());
        }
        return mAudioQueue.size();
    }

    @Override
    public int onReceiveVideo(MediaData inPacket) {
        if (mSourceState == SourceState.Init) {
            setSourceState(SourceState.Ready);
        }
        synchronized (mVideoQueueLock) {
            MediaData dstData = new MediaData(inPacket);
            long presentUs = inPacket.pts;
            dstData.pts = presentUs;
            mVideoQueue.add(dstData);
            if (VERBOSE) LogUtils.i(TAG, "receive new video package " + presentUs + ", size = " + mVideoQueue.size());
        }
        if (mOnSourceSizeChangedListener != null) {
            mOnSourceSizeChangedListener.onLocalVideoSizeChanged(mVideoQueue.size());
        }
        return mVideoQueue.size();
    }

    @Override
    public void onRelease() {
        LogUtils.e(TAG, "CoreFlow : onRelease");
        mIsSourceEnd = true;
    }

    @Override
    public void onError(int errorCode, String errorMessage) {
        LogUtils.e(TAG, "onError " + errorCode + " " + errorMessage);
        setSourceState(SourceState.Error);
        if (mOnErrorListener != null) {
            mOnErrorListener.onError(errorCode, errorMessage);
        }
    }

    @Override
    public void onFinishing() {
        setSourceState(SourceState.Finishing);
    }

    @Override
    public String getUrl() {
        return mUrl;
    }

    @Override
    public void setUrl(int type, String url) {
        if (TextUtils.isEmpty(url)) {
            return;
        }
        if (SOURCE_TYPE_FILE == type) {
            mUrl = "file:" + url;
        } else if (SOURCE_TYPE_P2P == type) {
            mUrl = "p2p:" + url;
        } else {
            mUrl = "file:" + url;
        }
    }

    @Override
    public void addFlag(int flag) {
        mFlag |= flag;
    }

    @Override
    public int getFlag() {
        return mFlag;
    }

    private void setSourceState(SourceState sourceState) {
        synchronized (this) {
            if (mSourceState == sourceState) {
                return;
            }
            mSourceState = sourceState;
            if (mOnSourceStateChangedListener != null) {
                mOnSourceStateChangedListener.onSourceStateChanged(mSourceState);
            }
            LogUtils.i(TAG, "CoreFlow setSourceState " + mSourceState);
        }
    }

    @Override
    public void setOnSourceStateChangedListener(OnSourceStateChangedListener listener) {
        this.mOnSourceStateChangedListener = listener;
    }

    @Override
    public MediaInfo getMediaInfo() {
        return mMediaInfo;
    }

    @Override
    public MediaData readAudioSource() {
        synchronized (mAudioQueueLock) {
            MediaData mediaData;
            mediaData = mAudioQueue.peek();
            checkSourceEnd();
            return mediaData;
        }
    }

    @Override
    public void removeFirstAudioPackage() {
        synchronized (mAudioQueueLock) {
            if (!mAudioQueue.isEmpty()) {
                MediaData mediaData = mAudioQueue.peek();
                if (mediaData != null && mOnTimeChangedListener != null) {
                    mOnTimeChangedListener.onAudioTimeChanged(mediaData.pts);
                }
                mAudioQueue.remove();
            }
            if (mAudioQueue.isEmpty()) {
                if (VERBOSE) LogUtils.e(TAG, "audio buffer queue empty!");
            }
        }
        checkSourceEnd();
    }

    @Override
    public MediaData readVideoSource() {
        synchronized (mVideoQueueLock) {
            MediaData mediaData;
            mediaData = mVideoQueue.peek();
            checkSourceEnd();
            return mediaData;
        }
    }

    @Override
    public void removeFirstVideoPackage() {
        synchronized (mVideoQueueLock) {
            if (!mVideoQueue.isEmpty()) {
                MediaData mediaData = mVideoQueue.peek();
                if (mediaData != null && mOnTimeChangedListener != null) {
                    mOnTimeChangedListener.onVideoTimeChanged(mediaData.pts);
                }
                mVideoQueue.remove();
            }
            if (mVideoQueue.isEmpty()) {
                if (VERBOSE) LogUtils.e(TAG, "video buffer queue empty!");
            }
        }
        checkSourceEnd();
    }

    @Override
    public void clearAudioQueue() {
        synchronized (mAudioQueueLock) {
            mAudioQueue.clear();
        }
    }

    @Override
    public void clearVideoQueue() {
        synchronized (mVideoQueueLock) {
            mVideoQueue.clear();
        }
    }

    @Override
    public void flushBuffer() {
        LogUtils.i(TAG, "CoreFlow flushBuffer");
        clearAudioQueue();
        clearVideoQueue();
    }

    @Override
    public void checkSourceEnd() {
        boolean isFinishing = (mSourceState == SourceState.Finishing);
        boolean isEmpty = (mAudioQueue.isEmpty() && mVideoQueue.isEmpty());
        if (mIsSourceEnd && (isFinishing || isEmpty)) {
            LogUtils.e(TAG, "CoreFlow checkSourceEnd true");
            setSourceState(SourceState.Release);
        }
    }

    @Override
    public void onRemoteAudioSizeChanged(int size) {
        if (mOnSourceSizeChangedListener != null) {
            mOnSourceSizeChangedListener.onRemoteAudioSizeChanged(size);
        }
    }

    @Override
    public void onRemoteVideoSizeChanged(int size) {
        if (mOnSourceSizeChangedListener != null) {
            mOnSourceSizeChangedListener.onRemoteVideoSizeChanged(size);
        }
    }

    @Override
    public void setOnSourceSizeChangedListener(OnSourceSizeChangedListener listener) {
        mOnSourceSizeChangedListener = listener;
    }

    @Override
    public void setOnTimeChangedListener(OnTimeChangedListener listener) {
        mOnTimeChangedListener = listener;
    }

    @Override
    public void setOnErrorListener(OnErrorListener listener) {
        mOnErrorListener = listener;
    }

    @NonNull
    @Override
    public String toString() {
        return "MediaSourceImp{" +
                "mChannelId=" + mChannelId +
                ", mUrl='" + mUrl + '\'' +
                ", mFlag=" + mFlag +
                '}';
    }
}
