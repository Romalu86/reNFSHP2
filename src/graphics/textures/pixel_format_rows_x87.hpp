#pragma once
#include "nfs/graphics/textures/pixel_codec.hpp"
namespace nfs::graphics::surfaces::pixels {
// Each format constructor supplies a fixed byte step.
constexpr Dword format_pixel_bytes(Dword format) {
    switch (format) {
    case 20u:
        return 3;
    case 21u:
        return 4;
    case 22u:
        return 4;
    case 23u:
        return 2;
    case 24u:
        return 2;
    case 25u:
        return 2;
    case 26u:
        return 2;
    case 27u:
        return 1;
    case 28u:
        return 1;
    case 29u:
        return 2;
    case 30u:
        return 2;
    case 31u:
        return 4;
    case 34u:
        return 4;
    case 40u:
        return 2;
    case 41u:
        return 1;
    case 50u:
        return 1;
    case 51u:
        return 2;
    case 52u:
        return 1;
    case 60u:
        return 2;
    case 61u:
        return 2;
    case 62u:
        return 4;
    case 63u:
        return 4;
    case 64u:
        return 4;
    case 65u:
        return 4;
    case 67u:
        return 4;
    case 70u:
        return 2;
    case 909200416u:
        return 2;
    case 909200449u:
        return 4;
    case 909201952u:
        return 6;
    case 909201985u:
        return 8;
    }
    return 0;
}
template <Dword Format> void read_format_row(Codec *object, Dword y, Dword z, Colour *output);
template <Dword Format> void write_format_row(Codec *object, Dword y, Dword z, const Colour *input);
} // namespace nfs::graphics::surfaces::pixels
