#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <unistd.h>
#include "file_protocol.h"
#include <base/Log.h>
#include <android/log.h>

//#define DEBUG_INPUT_VIDEO_FRAME 1

#define ENABLE_BIT_STREAM_FILTER 1

//#define DEBUG_LOG 1

#ifdef DEBUG_INPUT_VIDEO_FRAME
FILE * audiofile = NULL;
FILE * videofile = NULL;
#endif

static uint64_t pts2timeus(AVRational time_base, int64_t pts) {
    return (uint64_t) (av_q2d(time_base) * pts * 1000000);
}

static void print_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag) {
#ifdef DEBUG_LOG
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    logD("%s, flags:%d, pts:%lld pts_time:%s dts_time:%s size:%d\n",
         tag, pkt->flags,
         pts2timeus(*time_base, pkt->pts),
         av_ts2timestr(pkt->pts, time_base), av_ts2timestr(pkt->dts, time_base),
         pkt->size);
#endif
}

int create_file_channel(const char *url, const struct MediaCallback callback) {
    in_filename = (char *) malloc(strlen(url) + 1);
    memcpy(in_filename, url, strlen(url));
    in_filename[strlen(url)] = '\0';
    file_media_callback = callback;
    int ret = pthread_create(&file_thread_id, NULL, (void *) start_demuxing, NULL);
    if (ret != 0) {
        logD("create thread error");
        return -1;
    }
    fileChannelId = 0;
    return fileChannelId;
}

int destroy_file_channel(int channelId) {
    logD("destroy_file_channel start");
    if (in_filename) {
        free(in_filename);
    }
    readingFrame = false;
    pthread_join(file_thread_id, NULL);
    logD("destroy_file_channel end");
    return 0;
}

void start_demuxing() {
    AVFormatContext *ifmt_ctx = NULL;
#ifdef ENABLE_BIT_STREAM_FILTER
    AVBSFContext *audio_abs_ctx = NULL;
    AVBSFContext *video_abs_ctx = NULL;
    const AVBitStreamFilter *audio_abs_filter = NULL;
    const AVBitStreamFilter *video_abs_filter = NULL;
#endif
    AVPacket pkt;
    int ret;
    int i;
    int stream_index = 0;
    int audio_stream_index = -1;
    int video_stream_index = -1;
    int *stream_mapping = NULL;
    int stream_mapping_size = 0;

    av_register_all();
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        logD("Could not open input file '%s'", in_filename);
        goto end;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        logD("Failed to retrieve input stream information");
        goto end;
    }

#ifdef DEBUG_INPUT_VIDEO_FRAME
    audiofile = fopen("/sdcard/Android/data/com.gibbs.gplayer/files/Movies/demuxing_audio.aac", "wb+");
    videofile = fopen("/sdcard/Android/data/com.gibbs.gplayer/files/Movies/demuxing_video.h264", "wb+");
#endif

    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    stream_mapping_size = ifmt_ctx->nb_streams;
    stream_mapping = av_mallocz_array((size_t) stream_mapping_size, sizeof(*stream_mapping));
    if (!stream_mapping) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    MediaInfo *header = malloc(sizeof(MediaInfo));
    header->duration = ifmt_ctx->duration / 1000;
    header->videoType = 0;
    header->audioType = 0;

    for (i = 0; i < stream_mapping_size; i++) {
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;

        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_mapping[i] = -1;
            logD("Error: invalid media type %d", in_codecpar->codec_type);
            continue;
        }
        if (in_codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (video_stream_index == -1) {
                video_stream_index = i;
            } else {
                logD("Error: video_stream_index has been set to %d", video_stream_index);
            }
            if (in_codecpar->codec_id == AV_CODEC_ID_H264) {
                header->videoType = 1;
            } else if (in_codecpar->codec_id == AV_CODEC_ID_MPEG4) {
                header->videoType = 2;
            } else if (in_codecpar->codec_id == AV_CODEC_ID_JPEG2000) {
                header->videoType = 3;
            } else if (in_codecpar->codec_id == AV_CODEC_ID_MJPEG) {
                header->videoType = 4;
            } else if (in_codecpar->codec_id == AV_CODEC_ID_HEVC) {
                header->videoType = 5;
            } else {
                logD("Error: not support video type %d", in_codecpar->codec_id);
            }
            header->videoWidth = in_codecpar->width;
            header->videoHeight = in_codecpar->height;
            header->videoFrameRate = (in_stream->r_frame_rate.num / in_stream->r_frame_rate.den + 0.5f);
            logD("r_frame_rate = %d|%d", in_stream->r_frame_rate.num, in_stream->r_frame_rate.den);
#ifdef ENABLE_BIT_STREAM_FILTER
            if (in_codecpar->codec_id == AV_CODEC_ID_HEVC) {
                video_abs_filter = av_bsf_get_by_name("hevc_mp4toannexb");
            } else if (in_codecpar->codec_id == AV_CODEC_ID_H264) {
                video_abs_filter = av_bsf_get_by_name("h264_mp4toannexb");
            }
            av_bsf_alloc(video_abs_filter, &video_abs_ctx);
            avcodec_parameters_copy(video_abs_ctx->par_in, in_codecpar);
            av_bsf_init(video_abs_ctx);
#else

#endif
        } else if (in_codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audio_stream_index == -1) {
                audio_stream_index = i;
            } else {
                logD("Error: audio_stream_index has been set to %d", audio_stream_index);
            }
            header->audioType = 101;
            header->audioProfile = in_codecpar->profile;
            header->audioMode = (int) in_codecpar->channel_layout;
            header->audioChannels = in_codecpar->channels;
            header->audioBitWidth = in_codecpar->format;
            header->audioSampleRate = in_codecpar->sample_rate;
            header->sampleNumPerFrame = in_codecpar->frame_size;
#ifdef ENABLE_BIT_STREAM_FILTER
//            audio_abs_filter = av_bsf_get_by_name("aac_adtstoasc");
//            av_bsf_alloc(audio_abs_filter, &audio_abs_ctx);
//            avcodec_parameters_copy(audio_abs_ctx->par_in, in_codecpar);
//            av_bsf_init(audio_abs_ctx);
            //init adts context
            ret = aac_decode_extradata(&mADTSContext, in_codecpar->extradata,
                                       in_codecpar->extradata_size);
            if (ret < 0) {
                logD("Error: fail to init adts context, code is %d", ret);
            }
            logD("mADTSContext write_adts=%d, objecttype=%d, channel_conf=%d, sample_rate_index=%d",
                 mADTSContext.write_adts, mADTSContext.objecttype, mADTSContext.channel_conf,
                 mADTSContext.sample_rate_index);
#else

#endif
        } else if (in_codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
            logD("Error: subtitle media type");
        }

        stream_mapping[i] = stream_index++;
    }

    logD("audio_stream_index = %d, video_stream_index = %d, stream_mapping_size = %d\n",
         audio_stream_index, video_stream_index, stream_mapping_size);

//    AVDictionaryEntry *tag = NULL;
//    while ((tag = av_dict_get(ifmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
//        logD("metadata %s=%s\n", tag->key, tag->value);

    logD("av_init\n");
    readingFrame = true;
    file_media_callback.av_init(fileChannelId, header);
    //send SPS PPS
    file_media_callback.av_feed_video(fileChannelId,
                                      ifmt_ctx->streams[video_stream_index]->codecpar->extradata,
                                      (uint32_t) ifmt_ctx->streams[video_stream_index]->codecpar->extradata_size,
                                      0, 0, 2);

    file_media_callback.av_feed_audio(fileChannelId,
                                      ifmt_ctx->streams[audio_stream_index]->codecpar->extradata,
                                      (uint32_t) ifmt_ctx->streams[audio_stream_index]->codecpar->extradata_size,
                                      0, 0, 2);

    uint32_t feedAudioResult = 0;
    uint32_t feedVideoResult = 0;
    while (readingFrame) {
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
            break;

        if (pkt.stream_index >= stream_mapping_size ||
            stream_mapping[pkt.stream_index] < 0) {
            av_packet_unref(&pkt);
            continue;
        }

        if (pkt.flags & AV_PKT_FLAG_DISCARD) {
            av_packet_unref(&pkt);
            continue;
        }

        pkt.stream_index = stream_mapping[pkt.stream_index];
        if (pkt.stream_index == audio_stream_index) {
#ifdef ENABLE_BIT_STREAM_FILTER
//            av_bsf_send_packet(audio_abs_ctx, &pkt);
//            while (av_bsf_receive_packet(audio_abs_ctx, &pkt) == 0) {
//                print_packet(ifmt_ctx, &pkt, "audio");
//                file_media_callback.av_feed_audio(fileChannelId, pkt.data, pkt.size, pkt.pts, 0);
//            }
            AVRational time_base = ifmt_ctx->streams[(&pkt)->stream_index]->time_base;
            uint8_t *audioData = malloc((size_t) (pkt.size + ADTS_HEADER_SIZE));
            //add adts header
            aac_set_adts_head(&mADTSContext, mADTSHeader, pkt.size);
            memcpy(audioData, mADTSHeader, ADTS_HEADER_SIZE);
            memcpy(audioData + ADTS_HEADER_SIZE, pkt.data, (size_t) pkt.size);
            print_packet(ifmt_ctx, &pkt, "audio");
            feedAudioResult = file_media_callback.av_feed_audio(fileChannelId, audioData,
                                                                (uint32_t) (pkt.size +
                                                                         ADTS_HEADER_SIZE),
                                                                pts2timeus(time_base, pkt.pts),
                                                                pts2timeus(time_base, pkt.dts),
                                                                0);
            free(audioData);
#else
            print_packet(ifmt_ctx, &pkt, "audio");
            AVRational time_base = ifmt_ctx->streams[(&pkt)->stream_index]->time_base;
            file_media_callback.av_feed_audio(fileChannelId, pkt.data, pkt.size,
                    pts2timeus(time_base, pkt.pts), pts2timeus(time_base, pkt.dts), 0);
#endif

#ifdef DEBUG_INPUT_VIDEO_FRAME
            if(audiofile && pkt.size > 0){
                fwrite(pkt.data, 1, pkt.size, audiofile);
            }
#endif
        } else if (pkt.stream_index == video_stream_index) {
#ifdef ENABLE_BIT_STREAM_FILTER
            av_bsf_send_packet(video_abs_ctx, &pkt);
            while (av_bsf_receive_packet(video_abs_ctx, &pkt) == 0) {
                print_packet(ifmt_ctx, &pkt, "video");
                AVRational time_base = ifmt_ctx->streams[(&pkt)->stream_index]->time_base;
                if (pkt.size <= 0) {
                    continue;
                }
                feedVideoResult = file_media_callback.av_feed_video(fileChannelId, pkt.data,
                                                                    (uint32_t) pkt.size,
                                                                    pts2timeus(time_base, pkt.pts),
                                                                    pts2timeus(time_base, pkt.dts),
                                                                    (pkt.flags & AV_PKT_FLAG_KEY));
            }
#else
            print_packet(ifmt_ctx, &pkt, "video");
            AVRational time_base = ifmt_ctx->streams[(&pkt)->stream_index]->time_base;
            file_media_callback.av_feed_video(fileChannelId, pkt.data, pkt.size,
                    pts2timeus(time_base, pkt.pts), pts2timeus(time_base, pkt.dts),
                    (pkt.flags & AV_PKT_FLAG_KEY));
#endif

#ifdef DEBUG_INPUT_VIDEO_FRAME
            if(videofile && pkt.size > 0){
                fwrite(pkt.data, 1, pkt.size, videofile);
            }
#endif
        }

        if (ret < 0) {
            logD("Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);

        if (header->videoFrameRate <= 0 || header->audioSampleRate <= 0 ||
            header->sampleNumPerFrame <= 0) {
            continue;
        }

        int frameRate =
                header->videoFrameRate > MAX_FRAME_RATE ? MAX_FRAME_RATE : header->videoFrameRate;
        int audioRate = header->audioSampleRate / header->sampleNumPerFrame;
        if (feedAudioResult > audioRate * MAX_AV_TIME &&
            feedVideoResult > frameRate * MAX_AV_TIME) {
            sleepTimeUs += SLEEP_TIME_US;
        } else {
            sleepTimeUs = 0;
        }
        if (sleepTimeUs > 0) {
#ifdef DEBUG_LOG
            logD("queue size %d %d, usleep for %ld\n", feedAudioResult, feedVideoResult,
                 sleepTimeUs);
#endif
            usleep((useconds_t) sleepTimeUs);
            sleepTimeUs = 0;
        }
    }

    logD("av_destroy\n");
    readingFrame = false;
    file_media_callback.av_destroy(fileChannelId);

    end:

    avformat_close_input(&ifmt_ctx);

    av_freep(&stream_mapping);

#ifdef ENABLE_BIT_STREAM_FILTER
//    av_bsf_free(&audio_abs_ctx);
//    audio_abs_ctx = NULL;
    av_bsf_free(&video_abs_ctx);
    video_abs_ctx = NULL;
#endif

#ifdef DEBUG_INPUT_VIDEO_FRAME
    fclose(audiofile);
    audiofile = NULL;
    fclose(videofile);
    videofile = NULL;
#endif

    if (ret < 0 && ret != AVERROR_EOF) {
        logD("Error occurred: %s\n", av_err2str(ret));
    }
    logD("ending");
}

bool is_demuxing(int channelId) {
    return readingFrame;
}

void logD(const char *fmt, ...) {
    char log_info[2040];
    char *buf = log_info;
    int ret, len = sizeof(log_info);

    va_list arglist;
    va_start(arglist, fmt);

    ret = vsnprintf(buf, len - 1, fmt, arglist);
    if (ret < 0) {
        buf[len - 1] = 0;
        buf[len - 2] = '\n';
    }

    va_end(arglist);

    __android_log_print(2, "demuxing", log_info, "");
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

