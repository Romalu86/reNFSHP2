#include "nfs/graphics/textures/shape_conversion.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"

namespace nfs::graphics::textures::shape {
namespace {

template <class Pixel, bool Swizzle>
bool reorder(void *destination, const void *source, int width, int height, int bits) {
    if (!destination || !source || width <= 0 || height <= 0)
        return false;
    auto *output = static_cast<Pixel *>(destination);
    const auto *input = static_cast<const Pixel *>(source);
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            const unsigned shift = bits == 32 ? 5 : bits == 16 ? 4 : 3;
            Dword tiled = Dword(static_cast<std::int32_t>(
                                  tar::swizzled_offset(x, y, width, height, bits)) >> shift);
            Dword linear = y * width + x;
            output[Swizzle ? tiled : linear] = input[Swizzle ? linear : tiled];
        }
    return true;
}

} // namespace


bool swizzle32(void *out, const void *in, int w, int h) {
    return reorder<Dword, true>(out, in, w, h, 32);
}
bool swizzle16(void *out, const void *in, int w, int h) {
    return reorder<std::uint16_t, true>(out, in, w, h, 16);
}
bool swizzle8(void *out, const void *in, int w, int h) {
    return reorder<Byte, true>(out, in, w, h, 8);
}
bool swizzle4(void *, const void *, int, int) {
    return true;
} // MOV EAX,1 / RET.


bool unswizzle32(void *out, const void *in, int w, int h) {
    return reorder<Dword, false>(out, in, w, h, 32);
}
bool unswizzle16(void *out, const void *in, int w, int h) {
    return reorder<std::uint16_t, false>(out, in, w, h, 16);
}
bool unswizzle8(void *out, const void *in, int w, int h) {
    return reorder<Byte, false>(out, in, w, h, 8);
}
bool unswizzle4(void *, const void *, int, int) {
    return true;
} // MOV EAX,1 / RET.

// Unsupported depth still sets the tiled flag.
Byte swizzle_levels(Dword *destination, Dword *source) {
    int width = static_cast<std::int16_t>(source[1]);
    int height = static_cast<std::int16_t>(source[1] >> 16);
    Dword levels = source[3] >> 28;
    Byte supported = 1;
    for (Dword level = 0; level <= levels && supported; ++level) {
        void *input = tar::image_level(source, 0, 0, level);
        void *output = tar::image_level(destination, 0, 0, level);
        switch (tar::image_depth(source)) {
        case 4:
            swizzle4(output, input, width, height);
            break;
        case 8:
            swizzle8(output, input, width, height);
            break;
        case 15:
        case 16:
            swizzle16(output, input, width, height);
            break;
        case 32:
            swizzle32(output, input, width, height);
            break;
        default:
            supported = 0;
            break;
        }
        width >>= 1;
        height >>= 1;
    }
    destination[3] |= 8192;
    return supported;
}


Dword *swizzle_shape(Dword *image) {
    Dword *output = copy_shape(image);
    if (output && !(output[3] & 8192))
        swizzle_levels(output, image);
    return output;
}
Dword *unswizzle_shape(Dword *image) {
    int width = static_cast<std::int16_t>(image[1]);
    int height = static_cast<std::int16_t>(image[1] >> 16);
    Dword levels = image[3] >> 28;
    Dword *output = copy_shape(image);
    if (!output)
        return nullptr;
    if (output[3] & 8192) {
        output[3] &= ~8192u;
        for (Dword level = 0; level <= levels; ++level) {
            void *input = tar::image_level(image, 0, 0, level);
            void *destination = tar::image_level(output, 0, 0, level);
            switch (tar::image_depth(image)) {
            case 4:
                unswizzle4(destination, input, width, height);
                break;
            case 8:
                unswizzle8(destination, input, width, height);
                break;
            case 15:
            case 16:
                unswizzle16(destination, input, width, height);
                break;
            case 32:
                unswizzle32(destination, input, width, height);
                break;
            }
            if (width)
                width >>= 1;
            if (height)
                height >>= 1;
        }
    }
    return output;
}

} // namespace nfs::graphics::textures::shape
