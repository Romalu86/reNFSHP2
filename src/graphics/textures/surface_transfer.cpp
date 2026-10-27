#include "nfs/graphics/textures/surface_conversion.hpp"
#include "nfs/graphics/textures/pixel_codec.hpp"
#include "nfs/graphics/textures/surface_filters.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
#include <cstring>
namespace nfs::graphics::surfaces {
template <class... Args> static void codec_call(Dword *codec, unsigned slot, Args... args) {
    using Call = void(__thiscall *)(Dword *, Args...);
    reinterpret_cast<const Call *>(codec[0])[slot](codec, args...);
}
static void dispose(Dword *codec) {
    codec_call(codec, 0, Dword(1));
}

Transfer *construct(Transfer *object) {
    object->destination = nullptr;
    object->source = nullptr;
    return object;
}
void destroy(Transfer *object) {
    if (object->destination)
        dispose(object->destination);
    if (object->source)
        dispose(object->source);
}
// Preserve every conversion attempt and its order, including failures.
int transfer(Transfer *object, Image *destination, Image *source, Dword filter) {
    object->source = object->destination = nullptr;
    object->filter = filter;
    if (!(filter & 65535) || (filter & 65535) > 5 || (filter & 4293918720u))
        return static_cast<int>(2289436780u);
    destination->words[16] = filter & 524288;
    int result = static_cast<int>(2147500037u);
    object->destination = pixels::create(destination);
    if (object->destination) {
        object->source = pixels::create(source);
        if (object->source) {
            result = pixels::source(reinterpret_cast<pixels::Codec *>(object->destination),
                                    reinterpret_cast<const pixels::Codec *>(object->source));
            if (result < 0)
                goto cleanup;
            result = copy(object);
            if (result < 0)
                result = convert_rows(object);
            if (result < 0)
                result = clip(object);
            if (result < 0)
                result = point(object);
            if (result < 0)
                result = half_2d(object);
            if (result < 0)
                result = half_3d(object);
            if (result < 0)
                result = linear_2d(object);
            if (result < 0)
                result = linear_3d(object);
            if (result < 0)
                result = triangle_2d(object);
            if (result < 0)
                result = triangle_3d(object);
            result = result >= 0 ? 0 : static_cast<int>(2147500037u);
        }
    }
cleanup:
    if (object->destination) {
        dispose(object->destination);
        object->destination = nullptr;
    }
    if (object->source) {
        dispose(object->source);
        object->source = nullptr;
    }
    return result;
}
// Palette precheck compares the first 256 words, as the instructions do.
int copy(Transfer *object) {
    Dword *destination = object->destination;
    Dword *source = object->source;
    if (destination[1] != source[1])
        return static_cast<int>(2147500037u);
    if (source[4] || destination[1046] != source[1046] || destination[1047] != source[1047] ||
        destination[1048] != source[1048])
        return static_cast<int>(2147500037u);
    if (!destination[3])
        return copy_blocks(object);
    if (destination[5] && destination + 12 != source + 12 &&
        std::memcmp(destination + 12, source + 12, 1024))
        return static_cast<int>(2147500037u);
    for (Dword z = 0; z < destination[1048]; ++z) {
        auto *out = reinterpret_cast<std::uint8_t *>(destination[6] + destination[1045] * z);
        auto *in = reinterpret_cast<const std::uint8_t *>(source[6] + source[1045] * z);
        for (Dword y = 0; y < destination[1047]; ++y) {
            copy_forward(out, in, destination[1049]);
            in += source[1044];
            out += destination[1044];
        }
    }
    return 0;
}
 
int convert_rows(Transfer *object) {
    Dword *destination = object->destination;
    Dword *source = object->source;
    if (destination[1046] != source[1046] || destination[1047] != source[1047] ||
        destination[1048] != source[1048])
        return static_cast<int>(2147500037u);
    void *row = gui::application_allocate(destination[1046] * 16);
    if (!row)
        return static_cast<int>(2147500037u);
    for (Dword z = 0; z < destination[1048]; ++z)
        for (Dword y = 0; y < destination[1047]; ++y) {
            codec_call(source, 1, y, z, row);
            codec_call(destination, 2, y, z, row);
        }
    gui::application_release(row);
    return 0;
}
// Source-width maximum and height/depth minima intentionally differ.
int clip(Transfer *object) {
    if (static_cast<std::uint8_t>(object->filter) != 1)
        return static_cast<int>(2147500037u);
    Dword *destination = object->destination;
    Dword *source = object->source;
    Dword width = destination[1046] > source[1046] ? destination[1046] : source[1046];
    Dword height = destination[1047] < source[1047] ? destination[1047] : source[1047];
    Dword depth = destination[1048] < source[1048] ? destination[1048] : source[1048];
    auto *row = static_cast<Dword *>(gui::application_allocate(width * 16));
    if (!row)
        return static_cast<int>(2147500037u);
    auto *empty = static_cast<Dword *>(gui::application_allocate(destination[1046] * 16));
    if (!empty) {
        gui::application_release(row);
        return static_cast<int>(2147500037u);
    }
    std::memset(row, 0, width * 16);
    std::memset(empty, 0, destination[1046] * 16);
    for (Dword z = 0; z < depth; ++z) {
        Dword y = 0;
        for (; y < height; ++y) {
            codec_call(source, 1, y, z, row);
            codec_call(destination, 2, y, z, row);
        }
        for (; y < destination[1047]; ++y)
            codec_call(destination, 2, y, z, empty);
    }
    for (Dword z = depth; z < destination[1048]; ++z)
        for (Dword y = 0; y < destination[1047]; ++y)
            codec_call(destination, 2, y, z, empty);
    gui::application_release(row);
    gui::application_release(empty);
    return 0;
}
// Nearest sampling uses 16.16 integer steps and a per-slice row cache.
int point(Transfer *object) {
    if (static_cast<std::uint8_t>(object->filter) != 2)
        return static_cast<int>(2147500037u);
    Dword *source = object->source;
    Dword *destination = object->destination;
    auto *input = static_cast<Dword *>(gui::application_allocate(source[1046] * 16));
    if (!input)
        return static_cast<int>(2147942414u);
    auto *output = static_cast<Dword *>(gui::application_allocate(destination[1046] * 16));
    if (!output) {
        gui::application_release(input);
        return static_cast<int>(2147942414u);
    }
    Dword step_x = (source[1046] << 16) / destination[1046],
         step_y = (source[1047] << 16) / destination[1047],
         step_z = (source[1048] << 16) / destination[1048];
    Dword source_z = 0;
    for (Dword z = 0; z < destination[1048]; ++z, source_z += step_z) {
        Dword previous = ~0u, source_y = 0;
        for (Dword y = 0; y < destination[1047]; ++y, source_y += step_y) {
            if ((previous ^ source_y) & 4294901760u) {
                codec_call(source, 1, source_y >> 16, source_z >> 16, input);
                previous = source_y;
            }
            Dword source_x = 0;
            for (Dword x = 0; x < destination[1046]; ++x, source_x += step_x)
                std::memcpy(output + x * 4, input + (source_x >> 16) * 4, 16);
            codec_call(destination, 2, y, z, output);
        }
    }
    gui::application_release(input);
    gui::application_release(output);
    return 0;
}
} // namespace nfs::graphics::surfaces
