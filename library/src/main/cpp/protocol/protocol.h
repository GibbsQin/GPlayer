#ifndef GPLAYER_PROTOCOL_H
#define GPLAYER_PROTOCOL_H


#include "remuxing.h"
#include <media/Media.h>

#define PROTOCOL_TYPE_STREAM 0
#define PROTOCOL_TYPE_FILE 1

pthread_t start_demuxing(const int type, const char *web_url, const FfmpegCallback callback,
                         MediaInfo *mediaInfo, const int *channelId);


#endif //GPLAYER_PROTOCOL_H
