//
// Created by Gibbs on 2020/12/5.
//

#ifndef GPLAYER_ADTSENC_H
#define GPLAYER_ADTSENC_H

#include <stdint.h>

#define ADTS_HEADER_SIZE 7

typedef struct ADTSContext {
    uint32_t write_adts;
    uint32_t objecttype;
    uint32_t sample_rate_index;
    uint32_t channel_conf;
} ADTSContext;

int create_adts_context(ADTSContext *adts, unsigned char *pbuf, int bufsize);

int insert_adts_head(ADTSContext *adts, unsigned char *buf, uint32_t size);

#endif //GPLAYER_ADTSENC_H
