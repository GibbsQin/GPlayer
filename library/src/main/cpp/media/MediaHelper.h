/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#ifndef GPLAYER_MEDIAHELPER_H
#define GPLAYER_MEDIAHELPER_H


#include <stdlib.h>
#include <media/MediaData.h>

class MediaHelper {
public:
    static void copy(MediaData *src, MediaData *des);
};


#endif //GPLAYER_MEDIAHELPER_H
