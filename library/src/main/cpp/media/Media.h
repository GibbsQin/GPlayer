#ifndef GPLAYER_MEDIA_H
#define GPLAYER_MEDIA_H

#define FLAG_KEY_FRAME 0x00000001
#define FLAG_KEY_EXTRA_DATA 0x00000002
#define FLAG_KEY_RENDERED 0x00000004

/**
 * 媒体数据结构
 */
typedef struct MediaData {
    uint8_t *data;
    uint32_t size;
    uint8_t *data1;
    uint32_t size1;
    uint8_t *data2;
    uint32_t size2;

    uint64_t pts;
    uint64_t dts;
    uint32_t width;
    uint32_t height;
    uint8_t flag;//与FLAG_KEY_相关
} MediaData;

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

typedef struct MediaCallback {
    void (*av_init)(int channelId, MediaInfo *header);

    uint32_t (*av_feed_audio)(int channelId, uint8_t *pInputBuf, uint32_t dwInputDataSize,
                              uint64_t u64InputPTS, uint64_t u64InputDTS, int flag);

    uint32_t (*av_feed_video)(int channelId, uint8_t *pInputBuf, uint32_t dwInputDataSize,
                              uint64_t u64InputPTS, uint64_t u64InputDTS, int flag);

    void (*av_destroy)(int channelId);

    void (*av_error)(int channelId, int code, char *msg);
} MediaCallback;

#endif //GPLAYER_MEDIA_H
