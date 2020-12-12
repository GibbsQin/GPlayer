#include <base/Log.h>
#include "MediaPipe.h"

#define TAG "MediaPipe"

std::map<long, GPlayer *> MediaPipe::sGPlayerMap;

FfmpegCallback MediaPipe::sFfmpegCallback;

void MediaPipe::av_format_init(int channel, FormatInfo *formatInfo) {
    GPlayer *targetPlayer = sGPlayerMap[channel];
    if (targetPlayer != nullptr) {
        targetPlayer->av_init(formatInfo);
    }
}

void MediaPipe::av_format_extradata_audio(int channel, AVFormatContext *ifmt_ctx,
                                          uint8_t *pInputBuf, uint32_t dwInputDataSize) {

}

void MediaPipe::av_format_extradata_video(int channel, AVFormatContext *ifmt_ctx,
                                          uint8_t *pInputBuf, uint32_t dwInputDataSize) {

}

uint32_t MediaPipe::av_format_feed_audio(int channel, AVFormatContext *ifmt_ctx, AVPacket *packet) {
    AVRational time_base = ifmt_ctx->streams[packet->stream_index]->time_base;
    GPlayer *targetPlayer = sGPlayerMap[channel];
    if (targetPlayer != nullptr) {
        AVPacket *pkt = av_packet_alloc();
        av_packet_ref(pkt, packet);
        pkt->pts = ffmpeg_pts2timeus(time_base, packet->pts);
        pkt->dts = ffmpeg_pts2timeus(time_base, packet->dts);
        return targetPlayer->av_feed_audio(pkt);
    }
    return 0;
}

uint32_t MediaPipe::av_format_feed_video(int channel, AVFormatContext *ifmt_ctx, AVPacket *packet) {
    AVRational time_base = ifmt_ctx->streams[packet->stream_index]->time_base;
    GPlayer *targetPlayer = sGPlayerMap[channel];
    if (targetPlayer != nullptr) {
        AVPacket *pkt = av_packet_alloc();
        av_packet_ref(pkt, packet);
        pkt->pts = ffmpeg_pts2timeus(time_base, packet->pts);
        pkt->dts = ffmpeg_pts2timeus(time_base, packet->dts);
        targetPlayer->av_feed_video(pkt);
    }
    return 0;
}

void MediaPipe::av_format_destroy(int channel, AVFormatContext *ifmt_ctx) {
    GPlayer *targetPlayer = sGPlayerMap[channel];
    if (targetPlayer != nullptr) {
        targetPlayer->av_destroy();
    }
}

void MediaPipe::av_format_error(int channel, int code, char *msg) {
    GPlayer *targetPlayer = sGPlayerMap[channel];
    if (targetPlayer != nullptr) {
        targetPlayer->av_error(code, msg);
    }
}

LoopFlag MediaPipe::av_format_loop_wait(int channelId, int64_t *seekUs) {
    GPlayer *targetPlayer = sGPlayerMap[channelId];
    if (targetPlayer != nullptr) {
        return targetPlayer->loopWait(seekUs);
    }
    return BREAK;
}

void MediaPipe::deleteFromMap(int channelId) {
    GPlayer *targetPlayer = MediaPipe::sGPlayerMap[channelId];
    delete targetPlayer;
    MediaPipe::sGPlayerMap[channelId] = nullptr;
    MediaPipe::sGPlayerMap.erase(static_cast<const long &>(channelId));
    LOGI(TAG, "CoreFlow : deleteFromMap channelId %d, current sGPlayerMap size = %d", channelId,
         MediaPipe::sGPlayerMap.size());
}

void MediaPipe::initFfmpegCallback() {
    MediaPipe::sFfmpegCallback.av_format_init = &(MediaPipe::av_format_init);
    MediaPipe::sFfmpegCallback.av_format_extradata_audio = &(MediaPipe::av_format_extradata_audio);
    MediaPipe::sFfmpegCallback.av_format_extradata_video = &(MediaPipe::av_format_extradata_video);
    MediaPipe::sFfmpegCallback.av_format_feed_audio = &(MediaPipe::av_format_feed_audio);
    MediaPipe::sFfmpegCallback.av_format_feed_video = &(MediaPipe::av_format_feed_video);
    MediaPipe::sFfmpegCallback.av_format_destroy = &(MediaPipe::av_format_destroy);
    MediaPipe::sFfmpegCallback.av_format_error = &(MediaPipe::av_format_error);
    MediaPipe::sFfmpegCallback.av_format_loop_wait = &(MediaPipe::av_format_loop_wait);
}
