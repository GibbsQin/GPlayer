#ifndef GPLAYER_CODECUTILS_H
#define GPLAYER_CODECUTILS_H


#include <codec/ffmpeg/libavutil/samplefmt.h>
#include <media/Media.h>
#include <string>

class CodecUtils {
public:
    static int codecType2CodecId(int type);

    static std::string codecType2Mime(int type);

    static AVSampleFormat getSampleFormat(MediaInfo *header);

    static uint64_t getChannelLayout(MediaInfo *header);
};


#endif //GPLAYER_CODECUTILS_H
