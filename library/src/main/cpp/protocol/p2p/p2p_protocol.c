#include <malloc.h>
#include <stdbool.h>
#include "p2p_protocol.h"

int create_p2p_channel(const char *deviceId, const struct MediaCallback callback) {
    mAVCallback = callback;
    return 0;
}

int destroy_p2p_channel(uint32_t link_chn_id) {
    return 0;
}

bool is_p2p_connecting(uint32_t link_chn_id) {
    return false;
}

int
iv_decode_audio(uint32_t link_chn_id, uint8_t *pInputBuf, uint32_t dwInputDataSize,
                uint64_t u64InputPTS) {
    return mAVCallback.av_feed_audio(link_chn_id, pInputBuf, dwInputDataSize, u64InputPTS,
                                     u64InputPTS, 0);
}

int
iv_decode_video(uint32_t link_chn_id, uint8_t *pInputBuf, uint32_t dwInputDataSize,
                uint64_t u64InputPTS) {
    return mAVCallback.av_feed_video(link_chn_id, pInputBuf, dwInputDataSize, u64InputPTS,
                                     u64InputPTS, 0);
}

void iv_destroy_decoder(uint32_t link_chn_id) {
    mAVCallback.av_destroy(link_chn_id);
}
