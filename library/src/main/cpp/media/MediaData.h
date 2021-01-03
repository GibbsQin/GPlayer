/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#ifndef GPLAYER_MEDIADATA_H
#define GPLAYER_MEDIADATA_H

#include <string>

class MediaData {
public:
    MediaData();

    MediaData(uint32_t s, uint32_t s1, uint32_t s2);

    MediaData(uint8_t *d, uint32_t s, uint8_t *d1, uint32_t s1, uint8_t *d2, uint32_t s2);

    ~MediaData();

    void print() const;

public:
    uint8_t *data = nullptr;
    uint32_t size = 0;
    uint8_t *data1 = nullptr;
    uint32_t size1 = 0;
    uint8_t *data2 = nullptr;
    uint32_t size2 = 0;

    uint64_t pts = 0;
    uint64_t dts = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t flag = 0;//与FLAG_KEY_相关
};


#endif //GPLAYER_MEDIADATA_H
