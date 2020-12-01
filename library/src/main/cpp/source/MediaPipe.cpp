#include <base/Log.h>
#include "MediaPipe.h"

#define TAG "GPlayerMgr"

std::map<long, GPlayerEngine *> MediaPipe::sGPlayerMap;

FfmpegCallback MediaPipe::sFfmpegCallback;

void MediaPipe::av_format_init(int channel, AVFormatContext *ifmt_ctx,
                               AVStream *audioStream, AVStream *videoStream, MediaInfo *mediaInfo) {
    ffmpeg_extra_audio_info(ifmt_ctx, audioStream, mediaInfo);
    ffmpeg_extra_video_info(ifmt_ctx, videoStream, mediaInfo);
    av_init(channel, mediaInfo);
}

void MediaPipe::av_format_extradata_audio(int channel, AVFormatContext *ifmt_ctx,
                                          uint8_t *pInputBuf, uint32_t dwInputDataSize) {
    av_feed_audio(channel, pInputBuf, dwInputDataSize, 0, 0, 2);
}

void MediaPipe::av_format_extradata_video(int channel, AVFormatContext *ifmt_ctx,
                                          uint8_t *pInputBuf, uint32_t dwInputDataSize) {
    av_feed_video(channel, pInputBuf, dwInputDataSize, 0, 0, 2);
}

uint32_t MediaPipe::av_format_feed_audio(int channel, AVFormatContext *ifmt_ctx, AVPacket *packet) {
    AVRational time_base = ifmt_ctx->streams[packet->stream_index]->time_base;
    return av_feed_audio(channel, packet->data, packet->size,
                         ffmpeg_pts2timeus(time_base, packet->pts),
                         ffmpeg_pts2timeus(time_base, packet->dts), 0);
}

uint32_t MediaPipe::av_format_feed_video(int channel, AVFormatContext *ifmt_ctx, AVPacket *packet) {
    AVRational time_base = ifmt_ctx->streams[packet->stream_index]->time_base;
    return av_feed_video(channel, packet->data, packet->size,
                         ffmpeg_pts2timeus(time_base, packet->pts),
                         ffmpeg_pts2timeus(time_base, packet->dts),
                         (packet->flags & AV_PKT_FLAG_KEY));
}

void MediaPipe::av_format_destroy(int channel, AVFormatContext *ifmt_ctx) {
    av_destroy(channel);
}

void MediaPipe::av_format_error(int channel, int code, char *msg) {
    av_error(channel, code, msg);
}

uint32_t MediaPipe::av_format_loop_wait(int channelId) {
    GPlayerEngine *targetPlayer = sGPlayerMap[channelId];
    if (targetPlayer != nullptr) {
        return targetPlayer->isDemuxingLoop() ? 1 : 0;
    }
    return 0;
}

void MediaPipe::av_init(int channelId, MediaInfo *header) {
    GPlayerEngine *targetPlayer = sGPlayerMap[channelId];
    if (targetPlayer != nullptr && targetPlayer->getInputSource()) {
        targetPlayer->getInputSource()->onInit(header);
        targetPlayer->onInit();
    }
}

uint32_t MediaPipe::av_feed_audio(int channelId, uint8_t *pInputBuf, uint32_t dwInputDataSize,
                                  uint64_t u64InputPTS, uint64_t u64InputDTS, int flag) {
    auto *avData = new MediaData();
    avData->data = static_cast<uint8_t *>(malloc(dwInputDataSize));
    memcpy(avData->data, pInputBuf, dwInputDataSize);
    avData->size = dwInputDataSize;
    avData->pts = u64InputPTS;
    avData->dts = u64InputDTS;
    avData->flag = flag;

    GPlayerEngine *targetPlayer = sGPlayerMap[channelId];
    if (targetPlayer != nullptr && targetPlayer->getInputSource()) {
        return targetPlayer->getInputSource()->onReceiveAudio(avData);
    }

    return 0;
}

uint32_t MediaPipe::av_feed_video(int channelId, uint8_t *pInputBuf, uint32_t dwInputDataSize,
                                  uint64_t u64InputPTS, uint64_t u64InputDTS, int flag) {
    auto *avData = new MediaData();
    avData->data = static_cast<uint8_t *>(malloc(dwInputDataSize));
    memcpy(avData->data, pInputBuf, dwInputDataSize);
    avData->size = dwInputDataSize;
    avData->pts = u64InputPTS;
    avData->dts = u64InputDTS;
    avData->flag = flag;

    GPlayerEngine *targetPlayer = sGPlayerMap[channelId];
    if (targetPlayer != nullptr && targetPlayer->getInputSource()) {
        return targetPlayer->getInputSource()->onReceiveVideo(avData);
    }

    return 0;
}

void MediaPipe::av_destroy(int channelId) {
    GPlayerEngine *targetPlayer = sGPlayerMap[channelId];
    if (targetPlayer != nullptr && targetPlayer->getInputSource()) {
        targetPlayer->getInputSource()->onRelease();
    }
}

void MediaPipe::av_error(int channelId, int code, char *msg) {
    GPlayerEngine *targetPlayer = sGPlayerMap[channelId];
    if (targetPlayer != nullptr && targetPlayer->getOutputSource()) {
        targetPlayer->getOutputSource()->callJavaErrorMethod(code, msg);
    }
}

void MediaPipe::deleteFromMap(int channelId) {
    GPlayerEngine *targetPlayer = MediaPipe::sGPlayerMap[channelId];
    delete targetPlayer;
    MediaPipe::sGPlayerMap[channelId] = nullptr;
    MediaPipe::sGPlayerMap.erase(static_cast<const long &>(channelId));
    LOGE(TAG, "deleteFromMap channelId %d, current sGPlayerMap size = %d", channelId, MediaPipe::sGPlayerMap.size());
}
