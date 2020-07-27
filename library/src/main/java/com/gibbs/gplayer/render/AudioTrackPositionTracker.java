/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.gibbs.gplayer.render;

import android.media.AudioTimestamp;
import android.media.AudioTrack;
import android.os.Build;
import android.os.SystemClock;

import androidx.annotation.IntDef;
import androidx.annotation.Nullable;

import java.lang.annotation.Documented;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.reflect.Method;

/**
 * Wraps an {@link AudioTrack}, exposing a position based on {@link
 * AudioTrack#getPlaybackHeadPosition()} and {@link AudioTrack#getTimestamp(AudioTimestamp)}.
 *
 * <p>Call {@link #setAudioTrack(AudioTrack, int, int, int)} to set the audio track to wrap. Call
 * {@link #mayHandleBuffer(long)} if there is input data to write to the track. If it returns false,
 * the audio track position is stabilizing and no data may be written. Call {@link #start()}
 * immediately before calling {@link AudioTrack#play()}. Call {@link #pause()} when pausing the
 * track. Call {@link #handleEndOfStream(long)} when no more data will be written to the track. When
 * the audio track will no longer be used, call {@link #reset()}.
 */
/* package */ final class AudioTrackPositionTracker {

  /** {@link AudioTrack} playback states. */
  @Documented
  @Retention(RetentionPolicy.SOURCE)
  @IntDef({PLAYSTATE_STOPPED, PLAYSTATE_PAUSED, PLAYSTATE_PLAYING})
  private @interface PlayState {}
  /** @see AudioTrack#PLAYSTATE_STOPPED */
  private static final int PLAYSTATE_STOPPED = AudioTrack.PLAYSTATE_STOPPED;
  /** @see AudioTrack#PLAYSTATE_PAUSED */
  private static final int PLAYSTATE_PAUSED = AudioTrack.PLAYSTATE_PAUSED;
  /** @see AudioTrack#PLAYSTATE_PLAYING */
  private static final int PLAYSTATE_PLAYING = AudioTrack.PLAYSTATE_PLAYING;

  /**
   * AudioTrack timestamps are deemed spurious if they are offset from the system clock by more than
   * this amount.
   *
   * <p>This is a fail safe that should not be required on correctly functioning devices.
   */
  private static final long MAX_AUDIO_TIMESTAMP_OFFSET_US = 5 * 1000000L;

  /**
   * AudioTrack latencies are deemed impossibly large if they are greater than this amount.
   *
   * <p>This is a fail safe that should not be required on correctly functioning devices.
   */
  private static final long MAX_LATENCY_US = 5 * 1000000L;

  private static final long FORCE_RESET_WORKAROUND_TIMEOUT_MS = 200;

  private static final int MAX_PLAYHEAD_OFFSET_COUNT = 10;
  private static final int MIN_PLAYHEAD_OFFSET_SAMPLE_INTERVAL_US = 30000;
  private static final int MIN_LATENCY_SAMPLE_INTERVAL_US = 500000;

  private final long[] playheadOffsets;

  private @Nullable
  AudioTrack audioTrack;
  private int outputPcmFrameSize;
  private int bufferSize;
  private @Nullable AudioTimestampPoller audioTimestampPoller;
  private int outputSampleRate;
  private long bufferSizeUs;

  private long smoothedPlayheadOffsetUs;
  private long lastPlayheadSampleTimeUs;

  private @Nullable
  Method getLatencyMethod;
  private long latencyUs;

  private long lastLatencySampleTimeUs;
  private long lastRawPlaybackHeadPosition;
  private long rawPlaybackHeadWrapCount;
  private int nextPlayheadOffsetIndex;
  private int playheadOffsetCount;
  private long stopTimestampUs;
  private long forceResetWorkaroundTimeMs;
  private long stopPlaybackHeadPosition;
  private long endPlaybackHeadPosition;

  /**
   * Creates a new audio track position tracker.
   */
  public AudioTrackPositionTracker() {
    if (Build.VERSION.SDK_INT >= 18) {
      try {
        getLatencyMethod = AudioTrack.class.getMethod("getLatency", (Class<?>[]) null);
      } catch (NoSuchMethodException e) {
        // There's no guarantee this method exists. Do nothing.
      }
    }
    playheadOffsets = new long[MAX_PLAYHEAD_OFFSET_COUNT];
  }

  /**
   * Sets the {@link AudioTrack} to wrap. Subsequent method calls on this instance relate to this
   * track's position, until the next call to {@link #reset()}.
   *
   * @param audioTrack The audio track to wrap.
   * @param outputPcmFrameSize For PCM output encodings, the frame size. The value is ignored
   *     otherwise.
   * @param bufferSize The audio track buffer size in bytes.
   */
  public void setAudioTrack(
      AudioTrack audioTrack,
      int outputPcmFrameSize,
      int bufferSize) {
    this.audioTrack = audioTrack;
    this.outputPcmFrameSize = outputPcmFrameSize;
    this.bufferSize = bufferSize;
    audioTimestampPoller = new AudioTimestampPoller(audioTrack);
    outputSampleRate = audioTrack.getSampleRate();
    bufferSizeUs = framesToDurationUs(bufferSize / outputPcmFrameSize);
    lastRawPlaybackHeadPosition = 0;
    rawPlaybackHeadWrapCount = 0;
    stopTimestampUs = Long.MIN_VALUE + 1;
    forceResetWorkaroundTimeMs = Long.MIN_VALUE + 1;
    latencyUs = 0;
  }

  public long getCurrentPositionUs(boolean sourceEnded) {
    if (this.audioTrack.getPlayState() == PLAYSTATE_PLAYING) {
      maybeSampleSyncParams();
    }

    // If the device supports it, use the playback timestamp from AudioTrack.getTimestamp.
    // Otherwise, derive a smoothed position by sampling the track's frame position.
    long systemTimeUs = System.nanoTime() / 1000;
    if (audioTimestampPoller.hasTimestamp()) {
      // Calculate the speed-adjusted position using the timestamp (which may be in the future).
      long timestampPositionFrames = audioTimestampPoller.getTimestampPositionFrames();
      long timestampPositionUs = framesToDurationUs(timestampPositionFrames);
      if (!audioTimestampPoller.isTimestampAdvancing()) {
        return timestampPositionUs;
      }
      long elapsedSinceTimestampUs = systemTimeUs - audioTimestampPoller.getTimestampSystemTimeUs();
      return timestampPositionUs + elapsedSinceTimestampUs;
    } else {
      long positionUs;
      if (playheadOffsetCount == 0) {
        // The AudioTrack has started, but we don't have any samples to compute a smoothed position.
        positionUs = getPlaybackHeadPositionUs();
      } else {
        // getPlaybackHeadPositionUs() only has a granularity of ~20 ms, so we base the position off
        // the system clock (and a smoothed offset between it and the playhead position) so as to
        // prevent jitter in the reported positions.
        positionUs = systemTimeUs + smoothedPlayheadOffsetUs;
      }
      if (!sourceEnded) {
        positionUs -= latencyUs;
      }
      return positionUs;
    }
  }

  /** Starts position tracking. Must be called immediately before {@link AudioTrack#play()}. */
  public void start() {
    audioTimestampPoller.reset();
  }

  /** Returns whether the audio track is in the playing state. */
  public boolean isPlaying() {
    return audioTrack.getPlayState() == PLAYSTATE_PLAYING;
  }

  /**
   * Returns an estimate of the number of additional bytes that can be written to the audio track's
   * buffer without running out of space.
   *
   * <p>May only be called if the output encoding is one of the PCM encodings.
   *
   * @param writtenBytes The number of bytes written to the audio track so far.
   * @return An estimate of the number of bytes that can be written.
   */
  public int getAvailableBufferSize(long writtenBytes) {
    int bytesPending = (int) (writtenBytes - (getPlaybackHeadPosition() * outputPcmFrameSize));
    return bufferSize - bytesPending;
  }

  /** Returns whether the track is in an invalid state and must be recreated. */
  public boolean isStalled(long writtenFrames) {
    return forceResetWorkaroundTimeMs != Long.MIN_VALUE + 1
        && writtenFrames > 0
        && SystemClock.elapsedRealtime() - forceResetWorkaroundTimeMs
            >= FORCE_RESET_WORKAROUND_TIMEOUT_MS;
  }

  /**
   * Records the writing position at which the stream ended, so that the reported position can
   * continue to increment while remaining data is played out.
   *
   * @param writtenFrames The number of frames that have been written.
   */
  public void handleEndOfStream(long writtenFrames) {
    stopPlaybackHeadPosition = getPlaybackHeadPosition();
    stopTimestampUs = SystemClock.elapsedRealtime() * 1000;
    endPlaybackHeadPosition = writtenFrames;
  }

  /**
   * Returns whether the audio track has any pending data to play out at its current position.
   *
   * @param writtenFrames The number of frames written to the audio track.
   * @return Whether the audio track has any pending data to play out.
   */
  public boolean hasPendingData(long writtenFrames) {
    return writtenFrames > getPlaybackHeadPosition();
  }

  /**
   * Pauses the audio track position tracker, returning whether the audio track needs to be paused
   * to cause playback to pause. If {@code false} is returned the audio track will pause without
   * further interaction, as the end of stream has been handled.
   */
  public boolean pause() {
    resetSyncParams();
    if (stopTimestampUs == Long.MIN_VALUE + 1) {
      // The audio track is going to be paused, so reset the timestamp poller to ensure it doesn't
      // supply an advancing position.
      audioTimestampPoller.reset();
      return true;
    }
    // We've handled the end of the stream already, so there's no need to pause the track.
    return false;
  }

  /**
   * Resets the position tracker. Should be called when the audio track previous passed to {@link
   * #setAudioTrack(AudioTrack, int, int, int)} is no longer in use.
   */
  public void reset() {
    resetSyncParams();
    audioTrack = null;
    audioTimestampPoller = null;
  }

  private void maybeSampleSyncParams() {
    long playbackPositionUs = getPlaybackHeadPositionUs();
    if (playbackPositionUs == 0) {
      // The AudioTrack hasn't output anything yet.
      return;
    }
    long systemTimeUs = System.nanoTime() / 1000;
    if (systemTimeUs - lastPlayheadSampleTimeUs >= MIN_PLAYHEAD_OFFSET_SAMPLE_INTERVAL_US) {
      // Take a new sample and update the smoothed offset between the system clock and the playhead.
      playheadOffsets[nextPlayheadOffsetIndex] = playbackPositionUs - systemTimeUs;
      nextPlayheadOffsetIndex = (nextPlayheadOffsetIndex + 1) % MAX_PLAYHEAD_OFFSET_COUNT;
      if (playheadOffsetCount < MAX_PLAYHEAD_OFFSET_COUNT) {
        playheadOffsetCount++;
      }
      lastPlayheadSampleTimeUs = systemTimeUs;
      smoothedPlayheadOffsetUs = 0;
      for (int i = 0; i < playheadOffsetCount; i++) {
        smoothedPlayheadOffsetUs += playheadOffsets[i] / playheadOffsetCount;
      }
    }

    maybePollAndCheckTimestamp(systemTimeUs, playbackPositionUs);
    maybeUpdateLatency(systemTimeUs);
  }

  private void maybePollAndCheckTimestamp(long systemTimeUs, long playbackPositionUs) {
    if (!audioTimestampPoller.maybePollTimestamp(systemTimeUs)) {
      return;
    }

    // Perform sanity checks on the timestamp and accept/reject it.
    long audioTimestampSystemTimeUs = audioTimestampPoller.getTimestampSystemTimeUs();
    long audioTimestampPositionFrames = audioTimestampPoller.getTimestampPositionFrames();
    if (Math.abs(audioTimestampSystemTimeUs - systemTimeUs) > MAX_AUDIO_TIMESTAMP_OFFSET_US) {
      audioTimestampPoller.rejectTimestamp();
    } else if (Math.abs(framesToDurationUs(audioTimestampPositionFrames) - playbackPositionUs)
        > MAX_AUDIO_TIMESTAMP_OFFSET_US) {
      audioTimestampPoller.rejectTimestamp();
    } else {
      audioTimestampPoller.acceptTimestamp();
    }
  }

  private void maybeUpdateLatency(long systemTimeUs) {
    if (getLatencyMethod != null
        && systemTimeUs - lastLatencySampleTimeUs >= MIN_LATENCY_SAMPLE_INTERVAL_US) {
      try {
        // Compute the audio track latency, excluding the latency due to the buffer (leaving
        // latency due to the mixer and audio hardware driver).
        latencyUs =
                (Integer) getLatencyMethod.invoke(audioTrack)
                    * 1000L
                - bufferSizeUs;
        // Sanity check that the latency is non-negative.
        latencyUs = Math.max(latencyUs, 0);
        // Sanity check that the latency isn't too large.
        if (latencyUs > MAX_LATENCY_US) {
          latencyUs = 0;
        }
      } catch (Exception e) {
        // The method existed, but doesn't work. Don't try again.
        getLatencyMethod = null;
      }
      lastLatencySampleTimeUs = systemTimeUs;
    }
  }

  private long framesToDurationUs(long frameCount) {
    return (frameCount * 1000000L) / outputSampleRate;
  }

  private void resetSyncParams() {
    smoothedPlayheadOffsetUs = 0;
    playheadOffsetCount = 0;
    nextPlayheadOffsetIndex = 0;
    lastPlayheadSampleTimeUs = 0;
  }

  private long getPlaybackHeadPositionUs() {
    return framesToDurationUs(getPlaybackHeadPosition());
  }

  /**
   * {@link AudioTrack#getPlaybackHeadPosition()} returns a value intended to be interpreted as an
   * unsigned 32 bit integer, which also wraps around periodically. This method returns the playback
   * head position as a long that will only wrap around if the value exceeds {@link Long#MAX_VALUE}
   * (which in practice will never happen).
   *
   * @return The playback head position, in frames.
   */
  private long getPlaybackHeadPosition() {
    if (stopTimestampUs != Long.MIN_VALUE + 1) {
      // Simulate the playback head position up to the total number of frames submitted.
      long elapsedTimeSinceStopUs = (SystemClock.elapsedRealtime() * 1000) - stopTimestampUs;
      long framesSinceStop = (elapsedTimeSinceStopUs * outputSampleRate) / 1000000L;
      return Math.min(endPlaybackHeadPosition, stopPlaybackHeadPosition + framesSinceStop);
    }

    int state = audioTrack.getPlayState();
    if (state == PLAYSTATE_STOPPED) {
      // The audio track hasn't been started.
      return 0;
    }

    long rawPlaybackHeadPosition = 0xFFFFFFFFL & audioTrack.getPlaybackHeadPosition();

    if (Build.VERSION.SDK_INT <= 29) {
      if (rawPlaybackHeadPosition == 0
          && lastRawPlaybackHeadPosition > 0
          && state == PLAYSTATE_PLAYING) {
        // If connecting a Bluetooth audio device fails, the AudioTrack may be left in a state
        // where its Java API is in the playing state, but the native track is stopped. When this
        // happens the playback head position gets stuck at zero. In this case, return the old
        // playback head position and force the track to be reset after
        // {@link #FORCE_RESET_WORKAROUND_TIMEOUT_MS} has elapsed.
        if (forceResetWorkaroundTimeMs == Long.MIN_VALUE + 1) {
          forceResetWorkaroundTimeMs = SystemClock.elapsedRealtime();
        }
        return lastRawPlaybackHeadPosition;
      } else {
        forceResetWorkaroundTimeMs = Long.MIN_VALUE + 1;
      }
    }

    if (lastRawPlaybackHeadPosition > rawPlaybackHeadPosition) {
      // The value must have wrapped around.
      rawPlaybackHeadWrapCount++;
    }
    lastRawPlaybackHeadPosition = rawPlaybackHeadPosition;
    return rawPlaybackHeadPosition + (rawPlaybackHeadWrapCount << 32);
  }
}
