#include "movie_runtime.hpp"
namespace nfs::sound::streaming {

Dword enqueue_request(Dword stream, int delay, Dword request) {
    return enqueue(stream, delay, nullptr, int(request), 2);
}

int create_channel_stream(files::streaming::Channel *channel, const VoiceParameters *parameters,
                          int blocks, int packets, void *buffer, int bytes) {
    return create(parameters, blocks, packets, buffer, bytes, channel, 1);
}

int set_request_delay(Dword request, std::int16_t delay) {
    if (!control.initialized)
        return -10;
    int result = -8;
    lock();
    if (Block *block = find_block(request)) {
        block->delay = delay;
        result = 0;
    }
    unlock();
    return result;
}

int set_stream_rate(Dword handle, Dword rate) {
    if (!control.initialized)
        return -10;
    Stream *stream = lookup(handle);
    if (!stream)
        return -8;
    field<std::uint16_t>(&stream->voice_parameters, 12) = std::uint16_t(rate);
    set_voice_rate(Dword(stream->voice), rate);
    return 0;
}
} // namespace nfs::sound::streaming
