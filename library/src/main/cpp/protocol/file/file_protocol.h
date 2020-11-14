#ifndef GPLAYER_FILE_PROTOCOL_H
#define GPLAYER_FILE_PROTOCOL_H

#include <stdint.h>
#include <media/Media.h>
#include <stdbool.h>
#include <pthread.h>
#include <file_utils.h>

#define MAX_AV_TIME 2

#define SLEEP_TIME_US 100000

#define MAX_FRAME_RATE 24

int create_file_channel(const char *in_filename, const struct MediaCallback callback);

int destroy_file_channel(int channelId);

void start_file_demuxing();

bool is_file_demuxing(int channelId);

void logD(const char *fmt, ...);

static char *in_filename;
static struct MediaCallback file_media_callback;
static pthread_t file_thread_id;
static int fileChannelId;
static bool readingFrame;
static long sleepTimeUs = 0;

static unsigned char mADTSHeader[ADTS_HEADER_SIZE]={0};
static ADTSContext mADTSContext;

#endif //GPLAYER_FILE_PROTOCOL_H
