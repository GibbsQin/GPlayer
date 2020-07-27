#ifndef GPLAYER_P2P_PROTOCOL_H
#define GPLAYER_P2P_PROTOCOL_H

#include <stdint.h>
#include <media/Media.h>

MediaCallback mAVCallback;

int create_p2p_channel(const char *deviceId, const struct MediaCallback callback);

int destroy_p2p_channel(uint32_t link_chn_id);

bool is_p2p_connecting(uint32_t link_chn_id);

int iv_decode_audio(uint32_t link_chn_id, uint8_t *pInputBuf, uint32_t dwInputDataSize,
                    uint64_t u64InputPTS);

int iv_decode_video(uint32_t link_chn_id, uint8_t *pInputBuf, uint32_t dwInputDataSize,
                    uint64_t u64InputPTS);

void iv_destroy_decoder(uint32_t link_chn_id);

#endif //GPLAYER_P2P_PROTOCOL_H
