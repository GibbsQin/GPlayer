#ifndef GPLAYER_DEMUXING_H
#define GPLAYER_DEMUXING_H

#include "../codec/ffmpeg/libavcodec/avcodec.h"
#include "../codec/ffmpeg/libavformat/avformat.h"
#include "avformat_def.h"

uint64_t ffmpeg_pts2timeus(AVRational time_base, int64_t pts);

void ffmpeg_demuxing(char *filename, int channelId, FfmpegCallback callback, MediaInfo *mediaInfo);

void ffmpeg_extra_audio_info(AVFormatContext *ifmt_ctx, AVStream *stream, MediaInfo *mediaInfo);

void ffmpeg_extra_video_info(AVFormatContext *ifmt_ctx, AVStream *stream, MediaInfo *mediaInfo);

#endif //GPLAYER_DEMUXING_H
