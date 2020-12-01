
#include "avformat_def.h"

char *getMimeByCodeID(CODEC_TYPE type) {
    switch (type) {
        case CODEC_VIDEO_MPEG2VIDEO:
            return MIME_VIDEO_MPEG2VIDEO;
        case CODEC_VIDEO_H263:
            return MIME_VIDEO_H263;
        case CODEC_VIDEO_MPEG4:
            return MIME_VIDEO_MPEG4;
        case CODEC_VIDEO_AVC:
            return MIME_VIDEO_AVC;
        case CODEC_VIDEO_VP8:
            return MIME_VIDEO_VP8;
        case CODEC_VIDEO_VP9:
            return MIME_VIDEO_VP9;
        case CODEC_VIDEO_HEVC:
            return MIME_VIDEO_HEVC;
        case CODEC_AUDIO_G711_ALAW:
            return MIME_AUDIO_G711_ALAW;
        case CODEC_AUDIO_G711_MLAW:
            return MIME_AUDIO_G711_MLAW;
        case CODEC_AUDIO_AMR_NB:
            return MIME_AUDIO_AMR_NB;
        case CODEC_AUDIO_AMR_WB:
            return MIME_AUDIO_AMR_WB;
        case CODEC_AUDIO_MP3:
            return MIME_AUDIO_MP3;
        case CODEC_AUDIO_RAW_AAC:
            return MIME_AUDIO_RAW_AAC;
        case CODEC_AUDIO_VORBIS:
            return MIME_AUDIO_VORBIS;
        default:
            return "";
    }
}

