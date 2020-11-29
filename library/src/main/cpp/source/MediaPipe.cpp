#include <base/Log.h>
#include "MediaPipe.h"

#define TAG "GPlayerMgr"

std::map<long, GPlayerEngine *> MediaPipe::sGPlayerMap;

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
