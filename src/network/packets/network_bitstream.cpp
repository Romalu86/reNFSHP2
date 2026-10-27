#include "nfs/network/packets/network_bitstream.hpp"
#include <cstring>
namespace nfs::network::bits {
namespace {
std::int32_t signed_word(Dword value) {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
} // namespace
// Other bytes in the object are deliberately unwritten.
Stream *construct(Stream *stream) {
    stream->data[0] = 0;
    stream->written = 0;
    stream->read = 0;
    stream->writing = 1;
    return stream;
}

void reset(Stream *stream) {
    stream->data[0] = 0;
    stream->written = 0;
    stream->read = 0;
    stream->writing = 1;
}
// Forward DWORD copies, including their overlap behavior.
void bind(Stream *stream, const void *source, Dword bytes) {
    auto *input = static_cast<const std::uint8_t *>(source);
    Dword offset = 0;
    for (; offset < (bytes & ~3u); offset += 4)
        field<Dword>(stream, offset) = field<Dword>(const_cast<std::uint8_t *>(input), offset);
    for (; offset < bytes; ++offset)
        stream->data[offset] = input[offset];
    stream->written = bytes * 8;
    stream->writing = 0;
}
// Write the extra byte even when no bits remain.
void write(Stream *stream, Dword value, int width) {
    const Dword previous = stream->written;
    stream->written += Dword(width);
    const Dword offset = previous & 7;
    int index = static_cast<int>(previous) >> 3;
    Dword aligned = value << ((32u - std::uint8_t(width)) & 31);
    Dword remaining_value = aligned;
    int remaining = width;
    if (offset) {
        int count = 8 - offset;
        if (width < count)
            count = width;
        remaining_value = aligned << (std::uint8_t(count) & 31);
        remaining -= count;
        stream->data[index++] |= std::uint8_t(aligned >> ((offset + 24) & 31));
    }
    if (remaining < 9)
        stream->data[index] = std::uint8_t(remaining_value >> 24);
    else if (remaining != 0) {
        Dword count = ((Dword(remaining) - 1) >> 3) + 1;
        do {
            stream->data[index++] = std::uint8_t(remaining_value >> 24);
            remaining_value <<= 8;
        } while (--count);
    }
}
// No added bounds checks, padding, or shortened reads.
void read(Stream *stream, void *destination, int width) {
    const int previous = signed_word(stream->read);
    int index = previous / 8;
    const int offset = previous % 8;
    stream->read += Dword(width);
    int count = 8 - offset;
    if (width < count)
        count = width;
    int remaining = signed_word(Dword(width) - Dword(count));
    auto &output = *static_cast<Dword *>(destination);
    output = Dword(stream->data[index++]) << ((offset + 24) & 31);
    int shift = signed_word(24u - Dword(count));
    if (shift > 0) {
        Dword value = output;
        remaining = signed_word(Dword(remaining) +
                                (4294967295u - ((23u - Dword(count)) >> 3)) * 8u);
        do {
            value |= Dword(stream->data[index++]) << (shift & 31);
            shift -= 8;
        } while (shift > 0);
        output = value;
    }
    if (remaining > 0)
        output |= Dword(stream->data[index]) >> ((-shift) & 31);
    output >>= (32u - std::uint8_t(width)) & 31;
}
} // namespace nfs::network::bits
