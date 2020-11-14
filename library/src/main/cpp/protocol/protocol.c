#include <protocol/file/file_protocol.h>
#include <protocol/p2p/p2p_protocol.h>
#include <protocol/stream/stream_protocol.h>
#include "protocol.h"

int create_channel(const int type, const char *url, const struct MediaCallback callback) {
    if (type == PROTOCOL_TYPE_FILE) {
        return create_file_channel(url, callback);
    } else if (type == PROTOCOL_TYPE_P2P) {
        return create_p2p_channel(url, callback);
    } else {
        return create_stream_channel(url, callback);
    }
}

int destroy_channel(const int type, const int channelId) {
    if (type == PROTOCOL_TYPE_FILE) {
        return destroy_file_channel(channelId);
    } else if (type == PROTOCOL_TYPE_P2P) {
        return destroy_p2p_channel((uint32_t) channelId);
    } else {
        return destroy_stream_channel((uint32_t) channelId);
    }
}

bool is_channel_connecting(const int type, const int channelId) {
    if (type == PROTOCOL_TYPE_FILE) {
        return is_file_demuxing(channelId);
    } else if (type == PROTOCOL_TYPE_P2P) {
        return is_p2p_connecting(channelId);
    } else {
        return is_stream_demuxing(channelId);
    }
}

int sendCtlCmd(uint8_t *data) {
    return 0;
}
