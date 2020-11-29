#include <pthread.h>
#include "protocol.h"

pthread_t start_demuxing(const int type, const char *web_url, const FfmpegCallback callback,
                         MediaInfo *mediaInfo, const int *channelId) {
    pthread_t thread_id;
    DemuxingParams *params = (DemuxingParams *) malloc(sizeof(DemuxingParams));
    params->filename = web_url;
    params->callback = callback;
    params->channelId = channelId;
    params->mediaInfo = mediaInfo;
    pthread_create(&thread_id, NULL, (void *) ffmpeg_demuxing, params);
    return thread_id;
}
