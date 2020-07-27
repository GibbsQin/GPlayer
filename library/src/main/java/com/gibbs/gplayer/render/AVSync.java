package com.gibbs.gplayer.render;

public interface AVSync {
    long getNowUs();

    long getRealTimeUsForMediaTime(long mediaTimeUs);

    long getVsyncDurationNs();
}
