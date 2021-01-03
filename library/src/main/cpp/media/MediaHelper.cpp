/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#include <cstdlib>
#include <cstring>
#include "MediaHelper.h"

void MediaHelper::copy(MediaData *src, MediaData *des) {
    des->pts = src->pts;
    des->dts = src->dts;
    des->width = src->width;
    des->height = src->height;
    des->flag = src->flag;
    if (src->data != nullptr && src->size > 0) {
        des->data = static_cast<uint8_t *>(malloc(src->size));
        memcpy(des->data, src->data, src->size);
        des->size = src->size;
    }

    if (src->data1 != nullptr && src->size1 > 0) {
        des->data1 = static_cast<uint8_t *>(malloc(src->size1));
        memcpy(des->data1, src->data1, src->size1);
        des->size1 = src->size1;
    }

    if (src->data2 != nullptr && src->size2 > 0) {
        des->data2 = static_cast<uint8_t *>(malloc(src->size2));
        memcpy(des->data2, src->data2, src->size2);
        des->size2 = src->size2;
    }
}
