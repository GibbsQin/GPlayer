#include <base/Log.h>
#include "GPlayerMgr.h"

#define TAG "GPlayerMgr"

std::map<long, GPlayerImp *> GPlayerMgr::sGPlayerMap;

void GPlayerMgr::av_init(int channelId, MediaInfo *header) {
    GPlayerImp *targetPlayer = sGPlayerMap[channelId];
    if (targetPlayer != nullptr && targetPlayer->getInputSource()) {
        targetPlayer->getInputSource()->onInit(header);
        targetPlayer->onInit();
    }
}

uint32_t GPlayerMgr::av_feed_audio(int channelId, uint8_t *pInputBuf, uint32_t dwInputDataSize,
                                   uint64_t u64InputPTS, uint64_t u64InputDTS, int flag) {
    auto *avData = new MediaData();
    avData->data = static_cast<uint8_t *>(malloc(dwInputDataSize));
    memcpy(avData->data, pInputBuf, dwInputDataSize);
    avData->size = dwInputDataSize;
    avData->pts = u64InputPTS;
    avData->dts = u64InputDTS;
    avData->flag = flag;

    GPlayerImp *targetPlayer = sGPlayerMap[channelId];
    if (targetPlayer != nullptr && targetPlayer->getInputSource()) {
        return targetPlayer->getInputSource()->onReceiveAudio(avData);
    }

    return 0;
}

uint32_t GPlayerMgr::av_feed_video(int channelId, uint8_t *pInputBuf, uint32_t dwInputDataSize,
                                   uint64_t u64InputPTS, uint64_t u64InputDTS, int flag) {
    auto *avData = new MediaData();
    avData->data = static_cast<uint8_t *>(malloc(dwInputDataSize));
    memcpy(avData->data, pInputBuf, dwInputDataSize);
    avData->size = dwInputDataSize;
    avData->pts = u64InputPTS;
    avData->dts = u64InputDTS;
    avData->flag = flag;

    GPlayerImp *targetPlayer = sGPlayerMap[channelId];
    if (targetPlayer != nullptr && targetPlayer->getInputSource()) {
        return targetPlayer->getInputSource()->onReceiveVideo(avData);
    }

    return 0;
}

void GPlayerMgr::av_destroy(int channelId) {
    GPlayerImp *targetPlayer = sGPlayerMap[channelId];
    if (targetPlayer != nullptr && targetPlayer->getInputSource()) {
        targetPlayer->getInputSource()->onRelease();
    }
}

void GPlayerMgr::av_error(int channelId, int code, char *msg) {
    GPlayerImp *targetPlayer = sGPlayerMap[channelId];
    if (targetPlayer != nullptr && targetPlayer->getOutputSource()) {
        targetPlayer->getOutputSource()->callJavaErrorMethod(code, msg);
    }
}

void GPlayerMgr::deleteFromMap(int channelId) {
    GPlayerImp *targetPlayer = GPlayerMgr::sGPlayerMap[channelId];
    delete targetPlayer;
    GPlayerMgr::sGPlayerMap[channelId] = nullptr;
    GPlayerMgr::sGPlayerMap.erase(static_cast<const long &>(channelId));
    LOGE(TAG, "deleteFromMap channelId %d, current sGPlayerMap size = %d", channelId, GPlayerMgr::sGPlayerMap.size());
}
