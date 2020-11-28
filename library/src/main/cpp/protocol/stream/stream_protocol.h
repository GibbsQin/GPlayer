#ifndef GPLAYER_STREAM_PROTOCOL_H
#define GPLAYER_STREAM_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>
#include <media/Media.h>
#include <sys/types.h>
#include <codec/ffmpeg/libavformat/avformat.h>
#include "remuxing.h"

static void av_format_init(AVFormatContext *ifmt_ctx, AVStream *audioStream, AVStream *videoStream);

static void av_format_extradata_audio(AVFormatContext *ifmt_ctx, uint8_t *pInputBuf, uint32_t dwInputDataSize);

static void av_format_extradata_video(AVFormatContext *ifmt_ctx, uint8_t *pInputBuf, uint32_t dwInputDataSize);

static uint32_t av_format_feed_audio(AVFormatContext *ifmt_ctx, AVPacket *packet);

static uint32_t av_format_feed_video(AVFormatContext *ifmt_ctx, AVPacket *packet);

static void av_format_destroy(AVFormatContext *ifmt_ctx);

static void av_format_error(int code, char *msg);

int create_stream_channel(const char *in_filename, const struct MediaCallback callback);

int destroy_stream_channel(int channelId);

void start_stream_demuxing();

int is_stream_demuxing(int channelId);

int stream_loop();

static char *web_url;
static struct MediaCallback stream_media_callback;
static struct FfmpegCallback ffmpeg_callback;
static MediaInfo *mediaInfo;
static pthread_t stream_thread_id;
static int streamChannelId;
static int streamingFrame = false;
static long streamSleepTimeUs = 0;

#endif //GPLAYER_STREAM_PROTOCOL_H
