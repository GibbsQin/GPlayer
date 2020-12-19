//
// Created by qinshenghua on 2020/12/4.
//

#ifndef GPLAYER_MEDIAHELPER_H
#define GPLAYER_MEDIAHELPER_H


#include <stdlib.h>
#include <media/MediaData.h>

class MediaHelper {
public:
    static void copy(MediaData *src, MediaData *des);
};


#endif //GPLAYER_MEDIAHELPER_H
