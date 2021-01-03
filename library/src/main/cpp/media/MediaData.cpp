/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <base/Log.h>
#include "MediaData.h"
#include "MediaHelper.h"

MediaData::MediaData() = default;

MediaData::MediaData(uint32_t s, uint32_t s1, uint32_t s2) {
    if (s > 0) {
        data = static_cast<uint8_t *>(malloc(s));
    }
    if (s1 > 0) {
        data1 = static_cast<uint8_t *>(malloc(s1));
    }
    if (s2 > 0) {
        data2 = static_cast<uint8_t *>(malloc(s2));
    }
}

MediaData::MediaData(uint8_t *d, uint32_t s, uint8_t *d1, uint32_t s1, uint8_t *d2, uint32_t s2) {
    if (s > 0) {
        data = static_cast<uint8_t *>(malloc(s));
    }
    if (s1 > 0) {
        data1 = static_cast<uint8_t *>(malloc(s1));
    }
    if (s2 > 0) {
        data2 = static_cast<uint8_t *>(malloc(s2));
    }
    if (d != nullptr && s > 0) {
        memcpy(data, d, s);
        size = s;
    }
    if (d1 != nullptr && s1 > 0) {
        memcpy(data1, d1, s1);
        size1 = s1;
    }
    if (d2 != nullptr && s2 > 0) {
        memcpy(data2, d2, s2);
        size2 = s2;
    }
}

MediaData::~MediaData() {
    if (data != nullptr) {
        free(data);
        data = nullptr;
    }
    if (data1 != nullptr) {
        free(data1);
        data1 = nullptr;
    }
    if (data2 != nullptr) {
        free(data2);
        data2 = nullptr;
    }
}

void MediaData::print() const {
    LOGI("MediaData", "print MediaData %d %d %d", size, size1, size2);
}
