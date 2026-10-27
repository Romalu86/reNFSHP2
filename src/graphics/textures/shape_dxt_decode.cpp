#include "nfs/graphics/textures/shape_conversion.hpp"
#include <cstring>

namespace nfs::graphics::textures::shape {

// Only AL is defined by the return contract.
static Byte has_alpha(const Dword *blocks, int pixels, int format) {
    int count = pixels / 16;
    for (int block = 0; block < count; ++block, blocks += 4) {
        if (format == 3) {
            if (blocks[0] != 4294967295u || blocks[1] != 4294967295u)
                return 1;
        } else if ((blocks[0] & 65535) != 65535)
            return 1;
    }
    return 0;
}

// Preserve the integer decoder's distinct interpolation/biases.
void decode_colour_tile(Dword *output, int pitch, const Dword *input) {
    Dword endpoints = input[0];
    Dword red0 = endpoints & 63488, green0 = endpoints & 2016, blue0 = endpoints & 31;
    Dword blue1 = endpoints >> 2 & 507904, green1 = endpoints & 132120576;
    Dword first = ((red0 * 8 | green0) << 2 | blue0) << 3;
    Dword second = ((blue1 | green1) >> 3 | (endpoints & 4160749823u)) >> 8;
    Dword palette[4];
    Dword third;
    if (first < second) {
        palette[3] = 0;
        Dword average = (((green1 >> 11) + green0 * 32) & 130560) |
                       ((((endpoints & 4160749568u) >> 8) + red0 * 256) & 33423360) |
                       (((blue1 >> 11) + blue0 * 8) & 510);
        third = Dword(static_cast<int>(average << 2) >> 3) | 4278190080u;
    } else {
        Dword red1 = (endpoints & 4160749568u) >> 8;
        Dword weighted = ((blue0 * 40 + (blue1 >> 11) * 3) & 2040) |
                        ((red0 * 1280 + red1 * 3) & 133693440) |
                        ((green0 * 160 + (green1 >> 11) * 3) & 522240);
        third = Dword(static_cast<int>(weighted) >> 3);
        Dword other = ((blue0 * 24 + (blue1 >> 11) * 5) & 2040) |
                     ((green0 * 96 + (green1 >> 11) * 5) & 522240) | (red0 * 768 + red1 * 5);
        palette[3] = Dword(static_cast<int>(other) >> 3) | 4278190080u;
    }
    palette[0] = first | 4278452740u;
    palette[1] = second | 4278452740u;
    palette[2] = third | 4278190080u;
    const Byte *selectors = reinterpret_cast<const Byte *>(input + 1);
    for (unsigned row = 0; row < 4; ++row) {
        Byte bits = selectors[row];
        output[0] = palette[bits & 3];
        output[1] = palette[bits >> 2 & 3];
        Dword third_pixel = palette[bits >> 4 & 3];
        output[3] = palette[bits >> 6];
        output[2] = third_pixel;
        output = reinterpret_cast<Dword *>(reinterpret_cast<Dword>(output) + pitch);
    }
}

// Integer alpha endpoints use rounded division by five/seven.
void decode_alpha_tile(Dword *output, int pitch, const Byte *input, int format) {
    if (format == 3) {
        for (unsigned y = 0; y < 4; ++y) {
            for (unsigned pair = 0; pair < 2; ++pair) {
                Byte value = *input++;
                output[pair * 2] = Dword((value & 15) * 17) << 24 | (output[pair * 2] & 16777215);
                output[pair * 2 + 1] =
                    Dword((value >> 4) * 17) << 24 | (output[pair * 2 + 1] & 16777215);
            }
            output = reinterpret_cast<Dword *>(reinterpret_cast<Dword>(output) + pitch);
        }
        return;
    }
    Byte palette[8] = {input[0], input[1]};
    if (input[1] < input[0]) {
        for (unsigned i = 1; i < 7; ++i)
            palette[i + 1] = Byte((Dword(input[0]) * (7 - i) + Dword(input[1]) * i + 3) / 7);
    } else {
        for (unsigned i = 1; i < 5; ++i)
            palette[i + 1] = Byte((Dword(input[0]) * (5 - i) + Dword(input[1]) * i + 2) / 5);
        palette[6] = 0;
        palette[7] = 255;
    }
    const Byte *packed = input + 2;
    for (unsigned half = 0; half < 2; ++half, packed += 3) {
        Dword bits;
        std::memcpy(&bits, packed, 4);
        for (unsigned y = 0; y < 2; ++y) {
            auto *alpha = reinterpret_cast<Byte *>(output) + 3;
            for (unsigned x = 0; x < 4; ++x, bits >>= 3)
                alpha[x * 4] = palette[bits & 7];
            output = reinterpret_cast<Dword *>(reinterpret_cast<Dword>(output) + pitch);
        }
    }
}


void decode_dxt_image(const void *source, void *destination, int width, int height, int format) {
    const auto *input = static_cast<const Byte *>(source);
    auto *output = static_cast<Byte *>(destination);
    bool separate_alpha = format == 3 || format == 5;
    bool decode_alpha =
        separate_alpha && has_alpha(reinterpret_cast<const Dword *>(input), width * height, format);
    for (int y = 0; y < height; y += 4) {
        for (int x = 0; x < width; x += 4) {
            auto *tile = reinterpret_cast<Dword *>(output);
            decode_colour_tile(tile, width * 4,
                               reinterpret_cast<const Dword *>(input + (separate_alpha ? 8 : 0)));
            if (decode_alpha)
                decode_alpha_tile(tile, width * 4, input, format);
            output += 16;
            input += separate_alpha ? 16 : 8;
        }
        output += width * 12;
    }
}

} // namespace nfs::graphics::textures::shape
