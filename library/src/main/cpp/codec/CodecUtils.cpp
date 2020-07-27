//
// Created by Gibbs on 2020/4/26.
//

#include "CodecUtils.h"
#include "ffmpeg/libavcodec/avcodec.h"

int CodecUtils::codecType2CodecId(int type) {
    switch (type) {
        case VIDEO_TYPE_H264:
            return AV_CODEC_ID_H264;
        case VIDEO_TYPE_MPEG4:
            return AV_CODEC_ID_MPEG4;
        case VIDEO_TYPE_MJPEG:
            return AV_CODEC_ID_MJPEG;
        case VIDEO_TYPE_H265:
            return AV_CODEC_ID_HEVC;
        case AUDIO_TYPE_AAC:
            return AV_CODEC_ID_AAC;
        default:
            break;
    }
    return -1;
}

std::string CodecUtils::codecType2Mime(int type) {
    switch (type) {
        case VIDEO_TYPE_H264:
            return "video/avc";
        case VIDEO_TYPE_H265:
            return "video/hevc";
        case AUDIO_TYPE_AAC:
            return "audio/mp4a-latm";
        default:
            break;
    }
    return "";
}

AVSampleFormat CodecUtils::getSampleFormat(MediaInfo *header) {
    return (AVSampleFormat) header->audioBitWidth;
}

uint64_t CodecUtils::getChannelLayout(MediaInfo *header) {
    return static_cast<uint64_t>(header->audioMode);
}
