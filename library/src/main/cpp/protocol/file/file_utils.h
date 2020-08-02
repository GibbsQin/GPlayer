//
// Created by Gibbs on 2020/7/28.
//

#ifndef GPLAYER_FILE_UTILS_H
#define GPLAYER_FILE_UTILS_H

#define ADTS_HEADER_SIZE 7

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct {
    uint32_t write_adts;
    uint32_t objecttype;
    uint32_t sample_rate_index;
    uint32_t channel_conf;
} ADTSContext;

int aac_decode_extradata(ADTSContext *adts, unsigned char *pbuf, int bufsize);

int aac_set_adts_head(ADTSContext *acfg, unsigned char *buf, uint32_t size);

#endif //GPLAYER_FILE_UTILS_H
