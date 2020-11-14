#ifndef GPLAYER_PROTOCOL_H
#define GPLAYER_PROTOCOL_H


#include <stdint.h>
#include "media/Media.h"

#define PROTOCOL_TYPE_STREAM 0
#define PROTOCOL_TYPE_FILE 1

int create_channel(const int type, const char *url, const struct MediaCallback callback);

int destroy_channel(const int type, const int channelId);

bool is_channel_connecting(const int type, const int channelId);

int sendCtlCmd(uint8_t *data);


#endif //GPLAYER_PROTOCOL_H
