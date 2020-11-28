#ifndef GPLAYER_FILE_PROTOCOL_H
#define GPLAYER_FILE_PROTOCOL_H

#include <stdint.h>
#include <media/Media.h>
#include <stdbool.h>
#include <pthread.h>
#include <file_utils.h>
#include "stream_protocol.h"

static void av_format_init_file(AVFormatContext *ifmt_ctx, AVStream *audioStream, AVStream *videoStream);

static void av_format_extradata_audio_file(AVFormatContext *ifmt_ctx, uint8_t *pInputBuf, uint32_t dwInputDataSize);

static void av_format_extradata_video_file(AVFormatContext *ifmt_ctx, uint8_t *pInputBuf, uint32_t dwInputDataSize);

static uint32_t av_format_feed_audio_file(AVFormatContext *ifmt_ctx, AVPacket *packet);

static uint32_t av_format_feed_video_file(AVFormatContext *ifmt_ctx, AVPacket *packet);

static void av_format_destroy_file(AVFormatContext *ifmt_ctx);

static void av_format_error_file(int code, char *msg);

int create_file_channel(const char *in_filename, const struct MediaCallback callback);

int destroy_file_channel(int channelId);

void start_file_demuxing();

bool is_file_demuxing(int channelId);

int file_loop();

static char *file_url;
static struct MediaCallback file_media_callback;
static struct FfmpegCallback ffmpeg_callback;
static pthread_t file_thread_id;
static int fileChannelId;
static bool readingFrame;
static long fileSleepTimeUs = 0;

#endif //GPLAYER_FILE_PROTOCOL_H
