#include "adtsenc.h"

int create_adts_context(ADTSContext *adts, unsigned char *pbuf, int bufsize) {
    int aot;
    int aotext;
    int samfreindex;
    int channelconfig;
    unsigned char *p = pbuf;
    if (!adts || !pbuf || bufsize < 2) {
        return -1;
    }
    aot = (p[0] >> 3) & 0x1f;
    if (aot == 31) {
        aotext = (p[0] << 3 | (p[1] >> 5)) & 0x3f;
        aot = 32 + aotext;
        samfreindex = (p[1] >> 1) & 0x0f;
        if (samfreindex == 0x0f) {
            channelconfig = ((p[4] << 3) | (p[5] >> 5)) & 0x0f;
        } else {
            channelconfig = ((p[1] << 3) | (p[2] >> 5)) & 0x0f;
        }
    } else {
        samfreindex = ((p[0] << 1) | p[1] >> 7) & 0x0f;
        if (samfreindex == 0x0f) {
            channelconfig = (p[4] >> 3) & 0x0f;
        } else {
            channelconfig = (p[1] >> 3) & 0x0f;
        }
    }
#ifdef AOT_PROFILE_CTRL
    if (aot < 2) aot = 2;
#endif
    adts->objecttype = (uint32_t) (aot - 1);
    adts->sample_rate_index = (uint32_t) samfreindex;
    adts->channel_conf = (uint32_t) channelconfig;
    adts->write_adts = 1;
    return 0;
}

int insert_adts_head(ADTSContext *adts, unsigned char *buf, uint32_t size) {
    unsigned char byte;
    buf[0] = 0xff;
    buf[1] = 0xf1;
    byte = 0;
    byte |= (adts->objecttype & 0x03) << 6;
    byte |= (adts->sample_rate_index & 0x0f) << 2;
    byte |= (adts->channel_conf & 0x07) >> 2;
    buf[2] = byte;
    byte = 0;
    byte |= (adts->channel_conf & 0x07) << 6;
    byte |= (ADTS_HEADER_SIZE + size) >> 11;
    buf[3] = byte;
    byte = 0;
    byte |= (ADTS_HEADER_SIZE + size) >> 3;
    buf[4] = byte;
    byte = 0;
    byte |= ((ADTS_HEADER_SIZE + size) & 0x7) << 5;
    byte |= (0x7ff >> 6) & 0x1f;
    buf[5] = byte;
    byte = 0;
    byte |= (0x7ff & 0x3f) << 2;
    buf[6] = byte;
    return 0;
}

