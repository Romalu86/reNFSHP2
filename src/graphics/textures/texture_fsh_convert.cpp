#include "texture_fsh_convert.hpp"
#include "nfs/graphics/textures/shape_conversion.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
namespace nfs::graphics::textures {
using game::runtime::at;
using game::runtime::bytes;

Dword *convert_fsh_frame(TextureReader *reader, Dword *source, Dword *destination, Dword capacity,
                        std::uint8_t force_unpack, std::uint8_t atlas) {
    Dword *linear = source;
    const Dword type = *source & 127;
    if (!(atlas && at<short>(source, 4) == 1 && at<short>(source, 6) == 1)) {
        bool compressed = type == 96 || type == 97 || type == 98, supported = false;
        const auto format_for_type = [](Dword value) {
            for (const auto &row : formats)
                if (row.shape == value)
                    return row.format;
            return Dword(23);
        };
        if (compressed) {
            Dword format = format_for_type(type);
            if (format)
                supported = supports_format(reader, format);
        }
        if (force_unpack && (!compressed || !supported) && (std::int8_t(*source) < 0 || compressed))
            linear = shape::unpack_shape(source);
        const Dword unpacked_type = *linear;
        Dword selected;
        if (compressed && supported && !force_unpack)
            selected = format_for_type(unpacked_type & 127);
        else {
            const Dword transparency = alpha(linear, force_unpack);
            const Dword depth = tar::image_depth(linear);
            selected = fallback_format(depth, transparency);
        }
        const bool change_format = selected != format_for_type(unpacked_type & 127);
        const short source_width = at<short>(linear, 4), source_height = at<short>(linear, 6);
        Dword width = Dword(source_width), height = Dword(source_height);
        clamp_dimensions(reader, &width, &height);
        const auto *settings = reinterpret_cast<const std::uint8_t *>(reader->fields[7]);
        float scale = 1;
        if (compressed && !supported && settings[8])
            scale = 0.5f;
        if (settings[7])
            scale *= 0.5f;
        using kernel::numeric::Extended;
        const auto numeric = kernel::numeric::current_format();
        width = Dword(
            (Extended::integer(width, numeric) * Extended(scale, numeric)).truncate_to_integer());
        height = Dword(
            (Extended::integer(height, numeric) * Extended(scale, numeric)).truncate_to_integer());
        const bool resize = width != Dword(source_width) || height != Dword(source_height);
        if ((change_format || resize || settings[6]) && (std::int8_t(*linear) < 0 || compressed))
            linear = shape::unpack_shape(linear);
        if (change_format || resize || settings[6]) {
            Dword bits = 32;
            for (const auto &row : formats)
                if (row.format == selected) {
                    bits = row.bits;
                    break;
                }
            const Dword estimated = (((bits >> 3) * width + 15) & ~Dword(15)) * height;
            const Dword end = reinterpret_cast<Dword>(destination) + estimated + 80;
            if (end < reinterpret_cast<Dword>(destination) ||
                capacity < end - reinterpret_cast<Dword>(destination))
                return nullptr;
            Dword available = capacity;
            if (convert_shape_into(reader, linear, destination, selected, &available, scale)) {
                if (source != linear)
                    kernel::memory_free(linear);
                return nullptr;
            }
            if (source != linear)
                kernel::memory_free(linear);
            return shape_end(destination);
        }
    }
    const Dword size = reinterpret_cast<Dword>(shape_end(linear)) - reinterpret_cast<Dword>(linear);
    const Dword end = reinterpret_cast<Dword>(destination) + size;
    if (end < reinterpret_cast<Dword>(destination) ||
        capacity < end - reinterpret_cast<Dword>(destination))
        return nullptr;
    kernel::memory_copy(destination, linear, size);
    if (source != linear)
        kernel::memory_free(linear);
    return shape_end(destination);
}

Dword *convert_fsh_into(TextureReader *reader, const Dword *source, Dword *destination,
                       Dword capacity) {
    const Dword base = reinterpret_cast<Dword>(destination);
    kernel::memory_copy(destination, source, 24);
    Dword cursor = base + 16 + destination[2] * 8;
    if (cursor < base || capacity < cursor - base)
        return nullptr;
    const Dword count = source[2];
    const auto *settings = reinterpret_cast<const std::uint8_t *>(reader->fields[7]);
    const std::uint8_t force_unpack = settings[5];
    Dword single_pixel = 0;
    for (Dword index = 0; index < count; ++index) {
        auto *frame =
            reinterpret_cast<Dword *>(reinterpret_cast<Dword>(source) + source[5 + index * 2]);
        if (frame && at<short>(frame, 4) == 1 && at<short>(frame, 6) == 1)
            ++single_pixel;
    }
    const bool atlas = count > 1 && single_pixel == count - 1;
    float scale_x = 1, scale_y = 1;
    game::audio::vehicle::Progress progress;
    game::audio::vehicle::construct_progress(&progress, int(count));
    const auto finish = [&](Dword *result) {
        game::audio::vehicle::destroy_progress(&progress);
        return result;
    };
    try {
        for (Dword index = 0; index < count; ++index) {
            if (reinterpret_cast<const std::uint8_t *>(reader->fields[7])[9])
                game::audio::vehicle::advance_progress(&progress);
            Dword aligned = (cursor + 15) & ~Dword(15);
            auto *frame =
                reinterpret_cast<Dword *>(reinterpret_cast<Dword>(source) + source[5 + index * 2]);
            if (aligned + 5 < base || capacity < aligned + 5 - base || !frame)
                return finish(nullptr);
            destination[4 + index * 2] = source[4 + index * 2];
            destination[5 + index * 2] = aligned - base;
            auto *out = reinterpret_cast<Dword *>(aligned);
            Dword *end = convert_fsh_frame(reader, frame, out, base - aligned + capacity,
                                          force_unpack, std::uint8_t(atlas));
            cursor = reinterpret_cast<Dword>(end);
            if (!end)
                return finish(nullptr);
            using kernel::numeric::Extended;
            const auto numeric = kernel::numeric::current_format();
            if (at<short>(out, 4) != at<short>(frame, 4) ||
                at<short>(out, 6) != at<short>(frame, 6))
                scale_x = float(Extended::integer(at<short>(out, 4), numeric) /
                                Extended::integer(at<short>(frame, 4), numeric));
            if (at<short>(out, 6) != at<short>(frame, 6))
                scale_y = float(Extended::integer(at<short>(out, 6), numeric) /
                                Extended::integer(at<short>(frame, 6), numeric));
        }
        if (atlas &&
            ((std::isnan(scale_x) || scale_x != 1) || (std::isnan(scale_y) || scale_y != 1)))
            for (Dword index = 0; index < count; ++index) {
                auto *frame = reinterpret_cast<Dword *>(base + destination[5 + index * 2]);
                if (at<short>(frame, 4) == 1 && at<short>(frame, 6) == 1) {
                    using kernel::numeric::Extended;
                    const auto numeric = kernel::numeric::current_format();
                    const auto scaled = [&](int value, float scale) {
                        return Extended::integer(value, numeric) * Extended(scale, numeric);
                    };
                    at<short>(frame, 8) =
                        short(scaled(at<short>(frame, 8), scale_x).truncate_to_integer());
                    at<short>(frame, 10) =
                        short(scaled(at<short>(frame, 10), scale_y).truncate_to_integer());
                    int x = int(frame[3] << 20) >> 20;
                    float scaled_x = float(scaled(x, scale_x));
                    Dword new_x = Dword(Extended(std::ceil(scaled_x), numeric).truncate_to_integer());
                    frame[3] = ((new_x ^ frame[3]) & 4095) ^ frame[3];
                    int y = int(frame[3] << 4) >> 20;
                    float scaled_y = float(scaled(y, scale_y));
                    Dword new_y = Dword(Extended(std::ceil(scaled_y), numeric).truncate_to_integer());
                    frame[3] = (((new_y << 16) ^ frame[3]) & 268369920) ^ frame[3];
                    const int whole = int(Extended(scaled_y, numeric).truncate_to_integer());
                    const float fraction = scaled_y - float(whole);
                    if (std::isnan(fraction) || fraction != 0)
                        --at<short>(frame, 10);
                }
            }
    } catch (...) {
        game::audio::vehicle::destroy_progress(&progress);
        throw;
    }
    return finish(reinterpret_cast<Dword *>(cursor));
}
} // namespace nfs::graphics::textures
namespace nfs::game::runtime::particles::lifecycle::pending {
// The second saved stack argument is unused; RET8 retains it.
void *convert_fsh_image(graphics::TextureReader *reader, const void *source, Dword) {
    if (!graphics::context)
        return nullptr;
    auto *file = static_cast<const Dword *>(source);
    Dword *end;
    for (;;) {
        auto *buffer = reinterpret_cast<Dword *>(reader->fields[0]);
        if (buffer &&
            (end = graphics::textures::convert_fsh_into(reader, file, buffer, reader->fields[1])))
            break;
        if (graphics::textures::reserve(reader, reader->fields[1] << 1))
            return nullptr;
    }
    const Dword size = reinterpret_cast<Dword>(end) - reader->fields[0];
    void *result = kernel::memory_allocate(nullptr, int(size), 0);
    if (result) {
        kernel::memory_copy(result, reinterpret_cast<const void *>(reader->fields[0]), size);
        if (!size)
            files::file::save("C:\\guifsh.fsh", result, 0);
    }
    return result;
}
} // namespace nfs::game::runtime::particles::lifecycle::pending
