#include "texture_fsh_convert.hpp"
#include "nfs/graphics/textures/shape_conversion.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstring>
namespace nfs::graphics::textures {
using game::runtime::at;
using game::runtime::bytes;
namespace {
std::uint8_t *pixels(Dword *image) {
    return bytes(image) + ((image[3] & 4096) ? image[4] : 16);
}
Dword format_bits(Dword format) {
    for (const auto &row : formats)
        if (row.format == format)
            return row.bits;
    return 32;
}
Dword format_type(Dword format) {
    for (const auto &row : formats)
        if (row.format == format)
            return row.shape;
    return 120;
}
Dword image_format(Dword type) {
    for (const auto &row : formats)
        if (row.shape == type)
            return row.format;
    return 23;
}
} // namespace

static char *attached_shape_name(Dword *image) {
    while (image) {
        if (std::uint8_t(*image) == 111)
            return reinterpret_cast<char *>(image + 2);
        int step = int(*image) >> 8;
        if (!step)
            break;
        image = reinterpret_cast<Dword *>(reinterpret_cast<Dword>(image) + Dword(step));
    }
    return nullptr;
}

void copy_resized_shape(Dword *source, Dword *destination, Dword width, Dword height) {
    Dword source_width = Dword(at<short>(source, 4)), source_height = Dword(at<short>(source, 6));
    int depth = tar::image_depth(source);
    if (depth == 15)
        depth = 16;
    int source_pitch =
        int(std::uint8_t(*source) == 123 ? source_width * 8
                                         : (Dword(depth) * source_width + 7) & ~Dword(7)) >>
        3;
    int destination_pitch =
        int(std::uint8_t(*source) == 123 ? width * 8 : (Dword(depth) * width + 7) & ~Dword(7)) >> 3;
    auto *input = pixels(source);
    auto *output = pixels(destination);
    const Dword header = destination[3];
    at<std::uint16_t>(destination, 4) = std::uint16_t(width);
    at<std::uint16_t>(destination, 6) = std::uint16_t(height);
    if (source_width == width && source_height == height) {
        Dword levels = (header >> 28) & 31;
        Dword tail = (source_height >> levels) * (source_width >> levels);
        Dword size = (((source_height * source_width - tail) * 4) / 3 + tail) * Dword(depth >> 3);
        kernel::memory_copy(output, input, size);
    } else {
        Dword levels = (header >> 28) + 1;
        do {
            resize_pixels(output, input, int(width), int(height), int(source_width),
                          int(source_height), depth);
            input += Dword(source_pitch) * source_height;
            output += Dword(destination_pitch) * height;
            destination_pitch >>= 1;
            source_pitch >>= 1;
            source_width >>= 1;
            width >>= 1;
            source_height >>= 1;
            height >>= 1;
        } while (--levels);
    }
    Dword *palette = shape::palette(source);
    if (palette && shape::palette(destination))
        kernel::memory_copy(pixels(shape::palette(destination)), pixels(palette), 1024);
    if (char *name = attached_shape_name(source))
        std::strcpy(attached_shape_name(destination), name);
    if (Dword *metadata = tar::image_private(source))
        kernel::memory_copy(tar::image_private(destination), metadata, 64);
    *reinterpret_cast<std::uint8_t *>(destination) = std::uint8_t(*source);
}

static Dword *compress_shape(Dword *image) {
    return shape::pack_shape(image, tar::image_depth(image) > 25 ? 3 : 1, 100);
}

Dword convert_shape_into(TextureReader *reader, Dword *source, Dword *destination, Dword desired_format,
                        Dword *capacity, float scale) {
    Dword current_format = image_format(*source & 127);
    Dword *linear = source;
    const bool compressed = desired_format == 827611204 || desired_format == 861165636 ||
                            desired_format == 894720068;
    if (!compressed) {
        Dword type = format_type(desired_format);
        current_format = desired_format;
        if (type != (*source & 255))
            linear = shape::convert_type_default(source, type);
    }
    Dword width = Dword(at<short>(linear, 4)), height = Dword(at<short>(linear, 6));
    clamp_dimensions(reader, &width, &height);
    using kernel::numeric::Extended;
    const auto numeric = kernel::numeric::current_format();
    width =
        Dword((Extended::integer(width, numeric) * Extended(scale, numeric)).truncate_to_integer());
    height =
        Dword((Extended::integer(height, numeric) * Extended(scale, numeric)).truncate_to_integer());
    const int private_bytes = tar::image_private(source) ? 64 : 0;
    const char *name = attached_shape_name(source);
    Dword name_bytes = name ? Dword(std::strlen(name)) + 1 : 0;
    const int palette_depth = current_format == 41 ? 32 : 0;
    const Dword depth = format_bits(current_format);
    const Dword required = Dword(shape::allocation_bytes(
        int(width), int(height), int(depth), palette_depth, 0, int(name_bytes), private_bytes));
    if (*capacity < required) {
        if (linear != source)
            kernel::memory_free(linear);
        return 8;
    }
    shape::initialize_shape(destination, int(width), int(height), int(depth), palette_depth, 0,
                            name_bytes, private_bytes);
    shape_end(destination);
    copy_resized_shape(linear, destination, width, height);
    if (linear != source)
        kernel::memory_free(linear);
    const auto *settings = reinterpret_cast<const std::uint8_t *>(reader->fields[7]);
    if (settings[6] && !launch::field<Dword>(8627828)) {
        int minimum = at<short>(destination, 4);
        if (at<short>(destination, 6) <= minimum)
            minimum = at<short>(destination, 6);
        int levels = 0;
        if (!is_power(Dword(minimum))) {
            float value = float(minimum);
            int bits;
            std::memcpy(&bits, &value, 4);
            levels = (bits >> 23) - 127;
        } else
            for (; minimum > 8; minimum >>= 1)
                ++levels;
        Dword *mipped = shape::build_mips(destination, levels);
        if (!mipped)
            return 8;
        Dword size = reinterpret_cast<Dword>(shape_end(mipped)) - reinterpret_cast<Dword>(mipped);
        if (*capacity < size) {
            kernel::memory_free(mipped);
            return 8;
        }
        kernel::memory_copy(destination, mipped, size);
        kernel::memory_free(mipped);
    }
    if (compressed) {
        Dword *packed = compress_shape(destination);
        Dword size = reinterpret_cast<Dword>(shape_end(packed)) - reinterpret_cast<Dword>(packed);
        if (*capacity < size) {
            kernel::memory_free(packed);
            return 8;
        }
        kernel::memory_copy(destination, packed, size);
        kernel::memory_free(packed);
    }
    *capacity =
        reinterpret_cast<Dword>(shape_end(destination)) - reinterpret_cast<Dword>(destination);
    return 0;
}
} // namespace nfs::graphics::textures
