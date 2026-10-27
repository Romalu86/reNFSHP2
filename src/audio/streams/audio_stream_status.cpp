#include "nfs/audio/streams/audio_streams.hpp"
namespace nfs::sound::streaming {

std::uint64_t multiply_wide(Dword first, Dword second) {
    return std::uint64_t(first) * second;
}
// The restoring divider deliberately retains 32-bit wrap and divisor-zero
// behavior.
Dword divide_wide(Dword low, Dword high, Dword divisor) {
    Dword quotient = 0;
    for (unsigned bit = 0; bit < 32; ++bit) {
        quotient *= 2;
        Dword carry = low >> 31;
        low *= 2;
        high = high * 2 + carry;
        if (divisor <= high) {
            high -= divisor;
            ++quotient;
        }
    }
    return quotient;
}

int stream_status(Dword handle, Dword *status) {
    status[0] = status[1] = status[2] = 0;
    if (!control.initialized)
        return -10;
    Stream *stream = lookup(handle);
    if (!stream)
        return -8;
    status[0] = Dword(stream->playing.count);
    if (status[0]) {
        status[1] = stream->playing.first->handle;
        if (stream->active_format.sample_rate) {
            status[2] = (Dword(backend::pending::stream_samples(stream->decoder)) * 1000) /
                        stream->active_format.sample_rate;
            if (!status[2]) {
                Block *block = find_block(status[1]);
                if (block->byte_rate) {
                    Dword bytes = files::streaming::queued_bytes(stream->source);
                    if (bytes > 4000000)
                        bytes = 4000000;
                    status[2] = (bytes * 1000) / block->byte_rate;
                }
            }
        }
    }
    return 0;
}

int block_status(Dword handle, Dword *status) {
    for (unsigned i = 0; i < 4; ++i)
        status[i] = 0;
    if (!control.initialized)
        return -10;
    if (int(handle) < 0)
        return -8;
    Stream *stream = lookup(handle & 255);
    if (!stream)
        return -8;
    Block *block = find_block(handle);
    if (!block) {
        status[0] = 3;
        return 0;
    }
    if (!block->started) {
        status[0] = 0;
        return 0;
    }
    Dword rate;
    if (stream->playing.first == block) {
        status[0] = 2;
        rate = stream->active_format.sample_rate;
    } else {
        status[0] = 1;
        rate = stream->pending_format.sample_rate;
    }
    auto consumed = multiply_wide(block->consumed, 1000);
    status[1] = divide_wide(Dword(consumed), Dword(consumed >> 32), rate);
    auto remaining = multiply_wide(block->total - block->consumed, 1000);
    status[2] = divide_wide(Dword(remaining), Dword(remaining >> 32), rate);
    status[3] = (block->pending * 1000) / rate;
    return 0;
}
} // namespace nfs::sound::streaming
