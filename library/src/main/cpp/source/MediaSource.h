//
// Created by qinshenghua on 2020/12/15.
//

#ifndef GPLAYER_MEDIASOURCE_H
#define GPLAYER_MEDIASOURCE_H

#include <deque>
#include <mutex>

template<typename T>
class MediaSource {
public:
    uint32_t pushAudio(T data);

    uint32_t pushVideo(T data);

    uint32_t frontAudio(T **data);

    uint32_t frontVideo(T **data);

    void popAudio();

    void popVideo();

    void flush();

    uint32_t getAudioSize();

    uint32_t getVideoSize();

private:
    std::deque<T> audioQueue;
    std::deque<T> videoQueue;
    std::mutex audioLock;
    std::mutex videoLock;
};

template<typename T>
uint32_t MediaSource<T>::pushAudio(T data) {
    audioQueue.push_back(data);
    auto queueSize = static_cast<uint32_t>(audioQueue.size());
    return queueSize;
}

template<typename T>
uint32_t MediaSource<T>::pushVideo(T data) {
    videoQueue.push_back(data);
    auto queueSize = static_cast<uint32_t>(videoQueue.size());
    return queueSize;
}

template<typename T>
uint32_t MediaSource<T>::frontAudio(T **data) {
    if (audioQueue.size() <= 0) {
        return 0;
    }
    *data = audioQueue.front();
    if (!(*data)) {
        popAudio();
        return 0;
    }
    return static_cast<int>(audioQueue.size());
}

template<typename T>
uint32_t MediaSource<T>::frontVideo(T **data) {
    if (videoQueue.size() <= 0) {
        return 0;
    }
    *data = videoQueue.front();
    if (!(*data)) {
        popVideo();
        return 0;
    }
    return static_cast<int>(videoQueue.size());
}

template<typename T>
void MediaSource<T>::popAudio() {
    audioLock.lock();
    if (audioQueue.size() > 0) {
        MediaData *mediaData = audioQueue.front();
        delete mediaData;
        audioQueue.pop_front();
    }
    audioLock.unlock();
}

template<typename T>
void MediaSource<T>::popVideo() {
    videoLock.lock();
    if (videoQueue.size() > 0) {
        MediaData *mediaData = videoQueue.front();
        delete mediaData;
        videoQueue.pop_front();
    }
    videoLock.unlock();
}

template<typename T>
void MediaSource<T>::flush() {
    videoLock.lock();
    audioLock.lock();
    if (videoQueue.size() > 0) {
        videoQueue.clear();
    }
    if (audioQueue.size() > 0) {
        audioQueue.clear();
    }
    audioLock.unlock();
    videoLock.unlock();
}

template<typename T>
uint32_t MediaSource<T>::getAudioSize() {
    return static_cast<uint32_t>(audioQueue.size());
}

template<typename T>
uint32_t MediaSource<T>::getVideoSize() {
    return static_cast<uint32_t>(videoQueue.size());
}


#endif //GPLAYER_MEDIASOURCE_H
