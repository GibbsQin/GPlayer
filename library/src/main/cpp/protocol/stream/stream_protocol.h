#ifndef GPLAYER_STREAM_PROTOCOL_H
#define GPLAYER_STREAM_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>
#include <media/Media.h>
#include <sys/types.h>

typedef int (*is_demuxing)();

void start_demuxing(const char *in_filename, const struct MediaCallback media_callback,
                    const int channelId, is_demuxing loop);

int create_stream_channel(const char *in_filename, const struct MediaCallback callback);

int destroy_stream_channel(int channelId);

void start_stream_demuxing();

int is_stream_demuxing(int channelId);

int stream_loop();

static char *web_url;
static struct MediaCallback stream_media_callback;
static pthread_t stream_thread_id;
static int streamChannelId;
static int streamingFrame = false;

#endif //GPLAYER_STREAM_PROTOCOL_H
