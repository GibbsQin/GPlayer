//
// Created by qinshenghua on 2020/12/14.
//

#include "VideoRenderer.h"
#include "../media/MediaData.h"

VideoRenderer::VideoRenderer(OutputSource *source) {
    glesProgram = new YuvGlesProgram();
    mediaSource = source;
}

VideoRenderer::~VideoRenderer() {
    delete glesProgram;
    glesProgram = nullptr;
}

void VideoRenderer::init() {
    glesProgram->buildProgram();
}

void VideoRenderer::render(uint64_t nowMs) {
    MediaData *mediaData;
    mediaSource->readVideoBuffer(&mediaData);
    if (mediaData->pts < nowMs) {
        glesProgram->buildTextures(mediaData->data, mediaData->data1, mediaData->data2, mediaData->width, mediaData->height);
    }
}

void VideoRenderer::release() {

}
