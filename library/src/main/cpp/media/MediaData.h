//
// Created by qinshenghua on 2020/12/4.
//

#ifndef GPLAYER_MEDIADATA_H
#define GPLAYER_MEDIADATA_H


class MediaData {
public:
    MediaData(uint32_t s, uint32_t s1, uint32_t s2);

    MediaData(MediaData * src);

    MediaData(uint8_t *d, uint32_t s, uint8_t *d1, uint32_t s1, uint8_t *d2, uint32_t s2);

    ~MediaData();

public:
    uint8_t *data = nullptr;
    uint32_t size = 0;
    uint8_t *data1 = nullptr;
    uint32_t size1 = 0;
    uint8_t *data2 = nullptr;
    uint32_t size2 = 0;

    uint64_t pts;
    uint64_t dts;
    uint32_t width;
    uint32_t height;
    uint8_t flag;//与FLAG_KEY_相关
};


#endif //GPLAYER_MEDIADATA_H
