#ifndef GPLAYER_AVFORMAT_DEF_H
#define GPLAYER_AVFORMAT_DEF_H

#include "../codec/ffmpeg/libavcodec/avcodec.h"
#include "../codec/ffmpeg/libavformat/avformat.h"

typedef enum LoopFlag {
    LOOP,
    BREAK,
    CONTINUE,
} LoopFlag;

/**
 * 媒体信息
 */
typedef struct MediaInfo {
    int duration;           // 文件长度(毫秒)

    int audioType;          // 音频编码类型
    int audioCodecOption;   // 音频编码的参数
    int audioProfile;       // 音频编码规格
    int audioMode;          // 音频模式： 单声道/双声道
    int audioChannels;      // 音频声道数
    int audioBitWidth;      // 音频位宽
    int audioSampleRate;    // 音频采样率
    int sampleNumPerFrame;  // 每帧数据里的采样数

    int videoType;          // 视频编码类型
    int videoWidth;         // 视频像素宽度
    int videoHeight;        // 视频像素高度
    int videoFrameRate;     // 视频帧率
    int videoRotate;        // 视频旋转角度
} MediaInfo;

typedef struct FfmpegCallback {
    void (*av_format_init)(int channel, AVFormatContext *ifmt_ctx, AVStream *audioStream,
                           AVStream *videoStream, MediaInfo *mediaInfo);

    void (*av_format_extradata_audio)(int channel, AVFormatContext *ifmt_ctx, uint8_t *pInputBuf,
                                      uint32_t dwInputDataSize);

    void (*av_format_extradata_video)(int channel, AVFormatContext *ifmt_ctx, uint8_t *pInputBuf,
                                      uint32_t dwInputDataSize);

    uint32_t (*av_format_feed_audio)(int channel, AVFormatContext *ifmt_ctx, AVPacket *packet);

    uint32_t (*av_format_feed_video)(int channel, AVFormatContext *ifmt_ctx, AVPacket *packet);

    void (*av_format_destroy)(int channel, AVFormatContext *ifmt_ctx);

    void (*av_format_error)(int channel, int code, char *msg);

    LoopFlag (*av_format_loop_wait)(int channel);
} FfmpegCallback;

typedef enum CODEC_TYPE {
    CODEC_START              = 0,
    CODEC_VIDEO_MPEG2VIDEO   = AV_CODEC_ID_MPEG2VIDEO,//2
    CODEC_VIDEO_H263         = AV_CODEC_ID_H263,//224
    CODEC_VIDEO_MPEG4        = AV_CODEC_ID_MPEG4,//232
    CODEC_VIDEO_AVC          = AV_CODEC_ID_H264,//247
    CODEC_VIDEO_VP8          = AV_CODEC_ID_VP8,//360
    CODEC_VIDEO_VP9          = AV_CODEC_ID_VP9,//388
    CODEC_VIDEO_HEVC         = AV_CODEC_ID_HEVC,//394
    CODEC_AUDIO_G711_ALAW    = AV_CODEC_ID_PCM_ALAW,//445
    CODEC_AUDIO_G711_MLAW    = AV_CODEC_ID_PCM_ALAW + 1,
    CODEC_AUDIO_AMR_NB       = AV_CODEC_ID_AMR_NB,//533
    CODEC_AUDIO_AMR_WB       = AV_CODEC_ID_AMR_WB,//534
    CODEC_AUDIO_MP3          = AV_CODEC_ID_MP3,//550
    CODEC_AUDIO_RAW_AAC      = AV_CODEC_ID_AAC,//551
    CODEC_AUDIO_VORBIS       = AV_CODEC_ID_VORBIS,//554
    CODEC_END                = 0xffffffff,
} CODEC_TYPE;

#define MIME_VIDEO_MPEG2VIDEO  "video/mpeg2"           //- MPEG2 video
#define MIME_VIDEO_H263        "video/3gpp"            //- H.263 video
#define MIME_VIDEO_MPEG4       "video/mp4v-es"         //- MPEG4 video
#define MIME_VIDEO_AVC         "video/avc"             //- H.264/AVC video
#define MIME_VIDEO_VP8         "video/x-vnd.on2.vp8"   //- VP8 video (i.e. video in .webm)
#define MIME_VIDEO_VP9         "video/x-vnd.on2.vp9"   //- VP9 video (i.e. video in .webm)
#define MIME_VIDEO_HEVC        "video/hevc"            //- H.265/HEVC video
#define MIME_AUDIO_G711_ALAW   "audio/g711-alaw"       //- G.711 alaw audio
#define MIME_AUDIO_G711_MLAW   "audio/g711-mlaw"       //- G.711 ulaw audio
#define MIME_AUDIO_AMR_NB      "audio/3gpp"            //- AMR narrowband audio
#define MIME_AUDIO_AMR_WB      "audio/amr-wb"          //- AMR wideband audio
#define MIME_AUDIO_MP3         "audio/mpeg"            //- MPEG1/2 audio layer III
#define MIME_AUDIO_RAW_AAC     "audio/mp4a-latm"       //- AAC audio (note, this is raw AAC packets, not packaged in LATM!)
#define MIME_AUDIO_VORBIS      "audio/vorbis"          //- vorbis audio

char *getMimeByCodeID(CODEC_TYPE type);

#endif //GPLAYER_AVFORMAT_DEF_H
