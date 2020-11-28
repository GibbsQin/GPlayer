#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <unistd.h>
#include "file_protocol.h"
#include <base/Log.h>
#include <android/log.h>

#define ENABLE_BIT_STREAM_FILTER 1

#ifdef ENABLE_BIT_STREAM_FILTER
AVBSFContext *video_abs_ctx = NULL;
const AVBitStreamFilter *video_abs_filter = NULL;
unsigned char mADTSHeader[ADTS_HEADER_SIZE] = {0};
ADTSContext mADTSContext;
uint8_t *audioADTSData;
#endif

static void av_format_init_file(AVFormatContext *ifmt_ctx, AVStream *audioStream, AVStream *videoStream) {
    ffmpeg_extra_audio_info(ifmt_ctx, audioStream, mediaInfo);
    ffmpeg_extra_video_info(ifmt_ctx, videoStream, mediaInfo);
    file_media_callback.av_init(streamChannelId, mediaInfo);
#ifdef ENABLE_BIT_STREAM_FILTER
    AVCodecParameters *in_codecpar = audioStream->codecpar;
    create_adts_context(&mADTSContext, in_codecpar->extradata, in_codecpar->extradata_size);
    audioADTSData = malloc(in_codecpar->frame_size + ADTS_HEADER_SIZE);

    in_codecpar = videoStream->codecpar;
    if (in_codecpar->codec_id == AV_CODEC_ID_HEVC) {
        video_abs_filter = av_bsf_get_by_name("hevc_mp4toannexb");
    } else if (in_codecpar->codec_id == AV_CODEC_ID_H264) {
        video_abs_filter = av_bsf_get_by_name("h264_mp4toannexb");
    }
    av_bsf_alloc(video_abs_filter, &video_abs_ctx);
    avcodec_parameters_copy(video_abs_ctx->par_in, in_codecpar);
    av_bsf_init(video_abs_ctx);
#endif
}

static void av_format_extradata_audio_file(AVFormatContext *ifmt_ctx, uint8_t *pInputBuf,
                                           uint32_t dwInputDataSize) {
    file_media_callback.av_feed_audio(streamChannelId, pInputBuf, dwInputDataSize, 0, 0, 2);
}

static void av_format_extradata_video_file(AVFormatContext *ifmt_ctx, uint8_t *pInputBuf,
                                           uint32_t dwInputDataSize) {
    file_media_callback.av_feed_video(streamChannelId, pInputBuf, dwInputDataSize, 0, 0, 2);
}

static uint32_t av_format_feed_audio_file(AVFormatContext *ifmt_ctx, AVPacket *packet) {
    AVRational time_base = ifmt_ctx->streams[packet->stream_index]->time_base;
    int result = 0;
#ifdef ENABLE_BIT_STREAM_FILTER
    insert_adts_head(&mADTSContext, mADTSHeader, packet->size);
    memcpy(audioADTSData, mADTSHeader, ADTS_HEADER_SIZE);
    memcpy(audioADTSData + ADTS_HEADER_SIZE, packet->data, (size_t) packet->size);
    result = file_media_callback.av_feed_audio(streamChannelId, audioADTSData,
                                               (uint32_t) (packet->size + ADTS_HEADER_SIZE),
                                               ffmpeg_pts2timeus(time_base, packet->pts),
                                               ffmpeg_pts2timeus(time_base, packet->dts), 0);
#else
    result = file_media_callback.av_feed_audio(streamChannelId, packet->data, packet->size,
                                               ffmpeg_pts2timeus(time_base, packet->pts),
                                               ffmpeg_pts2timeus(time_base, packet->dts), 0);
#endif
    return result;
}

static uint32_t av_format_feed_video_file(AVFormatContext *ifmt_ctx, AVPacket *packet) {
    int result = 0;
#ifdef ENABLE_BIT_STREAM_FILTER
    av_bsf_send_packet(video_abs_ctx, packet);
    while (av_bsf_receive_packet(video_abs_ctx, packet) == 0) {
        AVRational time_base = ifmt_ctx->streams[packet->stream_index]->time_base;
        if (packet->size <= 0) {
            continue;
        }
        result = file_media_callback.av_feed_video(fileChannelId, packet->data,
                                                   (uint32_t) packet->size,
                                                   ffmpeg_pts2timeus(time_base, packet->pts),
                                                   ffmpeg_pts2timeus(time_base, packet->dts),
                                                   (packet->flags & AV_PKT_FLAG_KEY));
    }
#else
    AVRational time_base = ifmt_ctx->streams[packet->stream_index]->time_base;
    result = file_media_callback.av_feed_video(streamChannelId, packet->data, packet->size,
                                               ffmpeg_pts2timeus(time_base, packet->pts),
                                               ffmpeg_pts2timeus(time_base, packet->dts),
                                               (packet->flags & AV_PKT_FLAG_KEY));
#endif
    if (result > MAX_FRAME_RATE * MAX_AV_TIME) {
        fileSleepTimeUs += SLEEP_TIME_US;
    } else {
        fileSleepTimeUs = 0;
    }
    if (fileSleepTimeUs > 0) {
        usleep((useconds_t) fileSleepTimeUs);
        fileSleepTimeUs = 0;
    }
    return result;
}

static void av_format_destroy_file(AVFormatContext *ifmt_ctx) {
    file_media_callback.av_destroy(streamChannelId);

#ifdef ENABLE_BIT_STREAM_FILTER
    av_bsf_free(&video_abs_ctx);
    video_abs_ctx = NULL;
    free(audioADTSData);
    audioADTSData = NULL;
    free(mediaInfo);
    mediaInfo = NULL;
#endif
}

static void av_format_error_file(int code, char *msg) {
    file_media_callback.av_error(streamChannelId, code, msg);
}

int create_file_channel(const char *url, const struct MediaCallback callback) {
    file_url = (char *) malloc(strlen(url) + 1);
    memcpy(file_url, url, strlen(url));
    file_url[strlen(url)] = '\0';
    file_media_callback = callback;
    ffmpeg_callback.av_format_init = av_format_init_file;
    ffmpeg_callback.av_format_extradata_audio = av_format_extradata_audio_file;
    ffmpeg_callback.av_format_extradata_video = av_format_extradata_video_file;
    ffmpeg_callback.av_format_feed_audio = av_format_feed_audio_file;
    ffmpeg_callback.av_format_feed_video = av_format_feed_video_file;
    ffmpeg_callback.av_format_destroy = av_format_destroy_file;
    ffmpeg_callback.av_format_error = av_format_error_file;
    readingFrame = true;
    mediaInfo = malloc(sizeof(MediaInfo));
    mediaInfo->audioType = 0;
    mediaInfo->videoType = 0;
    int ret = pthread_create(&file_thread_id, NULL, (void *) start_file_demuxing, NULL);
    if (ret != 0) {
        return -1;
    }
    fileChannelId = 0;
    return fileChannelId;
}

int destroy_file_channel(int channelId) {
    if (file_url) {
        free(file_url);
    }
    readingFrame = false;
    pthread_join(file_thread_id, NULL);
    return 0;
}

void start_file_demuxing() {
    ffmpeg_demuxing(file_url, ffmpeg_callback, &file_loop);
}

bool is_file_demuxing(int channelId) {
    return readingFrame;
}

int file_loop() {
    return readingFrame;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

