#ifndef GPLAYER_FILE_PROTOCOL_H
#define GPLAYER_FILE_PROTOCOL_H

#include <stdint.h>
#include <media/Media.h>
#include <stdbool.h>
#include <pthread.h>

#define ADTS_HEADER_SIZE 7

#define MAX_AV_TIME 2

#define SLEEP_TIME_US 100000

#define MAX_FRAME_RATE 24

typedef struct {
    uint32_t write_adts;
    uint32_t objecttype;
    uint32_t sample_rate_index;
    uint32_t channel_conf;
} ADTSContext;

int create_file_channel(const char *in_filename, const struct MediaCallback callback);

int destroy_file_channel(int channelId);

void start_demuxing();

bool is_demuxing(int channelId);

int aac_decode_extradata(ADTSContext *adts, unsigned char *pbuf, int bufsize);

int aac_set_adts_head(ADTSContext *acfg, unsigned char *buf, uint32_t size);

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
