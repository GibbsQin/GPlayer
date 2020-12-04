package com.gibbs.gplayer.render;

import android.content.Context;

class AVSync {
    private AudioRender mAudioRender;
    private VideoFrameReleaseTimeHelper mFrameReleaseTimeHelper;
    private long mDeltaTimeUs;

    AVSync(Context context, AudioRender audioRender) {
        mAudioRender = audioRender;
        mFrameReleaseTimeHelper = new VideoFrameReleaseTimeHelper(context);
        mDeltaTimeUs = -1;
    }

    void enable() {
        mFrameReleaseTimeHelper.enable();
    }

    void disable() {
        mFrameReleaseTimeHelper.disable();
    }

    long getNowUs() {
        //the timestamp of audio playing
        if (mAudioRender == null) {
            return System.nanoTime() / 1000;
        }

        return mAudioRender.getAudioTimeUs();
    }

    long getRealTimeUsForMediaTime(long mediaTimeUs) {
        long nowUs = getNowUs();
        if (mDeltaTimeUs == -1) {
            mDeltaTimeUs = nowUs - mediaTimeUs;
        }
        long earlyUs = mDeltaTimeUs + mediaTimeUs - nowUs;
        long unadjustedFrameReleaseTimeNs = System.nanoTime() + (earlyUs * 1000);
        long adjustedReleaseTimeNs = mFrameReleaseTimeHelper.adjustReleaseTime(
                mDeltaTimeUs + mediaTimeUs, unadjustedFrameReleaseTimeNs);
        return adjustedReleaseTimeNs / 1000;
    }

    long getVsyncDurationNs() {
        if (mFrameReleaseTimeHelper != null) {
            return mFrameReleaseTimeHelper.getVsyncDurationNs();
        } else {
            return -1;
        }
    }
}
