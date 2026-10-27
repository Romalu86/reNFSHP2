#include "nfs/graphics/textures/shape_conversion.hpp"

namespace nfs::graphics::textures::shape {

AlternateUnpack alternate_unpack = nullptr;

// Copy backreferences byte by byte, including overlap.
int unpack_refpack(void *destination, const Byte *input, int *consumed) {
    const Byte *cursor = input;
    int decoded = 0;
    if (input) {
        Dword header = Dword(input[0]) * 256 + input[1];
        cursor = input + 2;
        Dword high;
        if (header & 32768) {
            if (header & 256)
                cursor = input + 6;
            high = Dword(cursor[0]) * 256 + cursor[1];
            ++cursor;
        } else {
            if (header & 256)
                cursor = input + 5;
            high = cursor[0];
        }
        decoded = (high * 256 + cursor[1]) * 256 + cursor[2];
        cursor += 3;
        auto *output = static_cast<Byte *>(destination);
        for (;;) {
            Dword tag = *cursor++;
            Dword literals, distance, length;
            if (tag < 128) {
                distance = (tag & 96) * 8 + *cursor++ + 1;
                literals = tag & 3;
                length = (tag >> 2 & 7) + 3;
            } else if (!(tag & 64)) {
                Dword next = *cursor++;
                distance = (next & 63) * 256 + *cursor++ + 1;
                literals = next >> 6;
                length = (tag & 63) + 4;
            } else if (!(tag & 32)) {
                distance = Dword(cursor[0]) * 256 + (tag & 16) * 4096 + cursor[1] + 1;
                length = cursor[2] + 5 + (tag & 12) * 64;
                cursor += 3;
                literals = tag & 3;
            } else {
                literals = (tag & 31) * 4 + 4;
                if (literals > 112) {
                    literals = tag & 3;
                    while (literals--)
                        *output++ = *cursor++;
                    break;
                }
                while (literals--)
                    *output++ = *cursor++;
                continue;
            }
            while (literals--)
                *output++ = *cursor++;
            const Byte *previous = output - distance;
            while (length--)
                *output++ = *previous++;
        }
    }
    if (consumed)
        *consumed =
            static_cast<int>(reinterpret_cast<Dword>(cursor) - reinterpret_cast<Dword>(input));
    return decoded;
}

// (five-byte tail JMP to ).
// Keep the optional alternate-codec registration explicit.
Dword unpack_buffer(const Byte *input, void *output) {
    if (input[1] != 251)
        return 0;
    switch (input[0] & 254) {
    case 16:
    case 144:
        return unpack_refpack(output, input, nullptr);
    case 30:
    case 158:
        if (alternate_unpack)
            return alternate_unpack(input, output);
    }
    return 0;
}

} // namespace nfs::graphics::textures::shape
