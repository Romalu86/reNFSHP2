#include "nfs/graphics/fonts/font_renderer.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/graphics/textures/shape_conversion.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cmath>
#include <cstring>
namespace nfs::graphics {
extern Dword font_shape_type;
}
namespace nfs::graphics::font_renderer {

Dword *convert_shape(Dword *source) {
    Dword *context = backend::selected_context(backend::selected_device());
    Dword preferred_format = nfs::graphics::state::context_backbuffer_format(context);
    context = backend::selected_context(backend::selected_device());
    int colour_depth = backend::color_depth(context);
    if (!font_shape_type)
        font_shape_type = tar::storage_format(preferred_format);
    Dword *converted = textures::shape::convert_type_default(source, font_shape_type);
    const auto *source_dimensions = reinterpret_cast<const std::int16_t *>(converted);
    int source_height = source_dimensions[3], width = source_dimensions[2];
    int height = source_height;
    if ((static_cast<Dword>(height) & (static_cast<Dword>(height) - 1)) != 0) {
        double candidate = 0;
        int exponent = 0;
        while (candidate < static_cast<float>(source_height))
            candidate = std::pow(2.0, static_cast<double>(++exponent));
        height = static_cast<int>(candidate);
    }
    int row_bytes = (width * colour_depth) / 8;
    int pixel_bytes = row_bytes * height;
    auto *allocation =
        static_cast<Dword *>(eagl::allocator(pixel_bytes + 24, "FONT_convertshapetype - Shape"));
    allocation[0] = pixel_bytes + 24;
    Dword *image = allocation + 1;
    kernel::memory_fill(image, 0, pixel_bytes + 20);
    auto *bytes = reinterpret_cast<std::uint8_t *>(image);
    bytes[pixel_bytes + 16] = 'C';
    bytes[pixel_bytes + 17] = 'S';
    bytes[pixel_bytes + 18] = 'H';
    bytes[pixel_bytes + 19] = 'P';
    reinterpret_cast<std::int16_t *>(image)[2] = static_cast<std::int16_t>(width);
    bytes[0] = static_cast<std::uint8_t>(font_shape_type);
    image[0] &= 255;
    reinterpret_cast<std::int16_t *>(image)[3] = static_cast<std::int16_t>(height);
    Dword flags = (image[3] & 268435455) | 268435456;
    flags ^= (converted[3] ^ flags) & 4095;
    flags ^= (converted[3] ^ flags) & 268369920;
    flags = ((flags ^ converted[3]) & 268435455) ^ converted[3];
    flags ^= (converted[3] ^ flags) & 8192;
    flags ^= (converted[3] ^ flags) & 16384;
    image[3] = flags;
    palette = tar::find_palette(converted);
    auto *input = reinterpret_cast<std::uint8_t *>(converted) +
                  ((converted[3] & 4096) ? converted[4] : 16);
    auto *output = bytes + ((image[3] & 4096) ? image[4] : 16);
    for (int row = 0; static_cast<float>(row) < static_cast<float>(source_height); ++row) {
        kernel::memory_copy(output, input, row_bytes);
        input += row_bytes;
        output += row_bytes;
    }
    kernel::memory_free(converted);
    return image;
}
} // namespace nfs::graphics::font_renderer
namespace nfs::graphics::tar {
// Unlike construct_shape, this path creates a managed texture.
Dword *create_image(Dword *image) {
    auto *object = static_cast<Dword *>(eagl::allocator(48, "EAGL::TAR new"));
    if (object)
        construct(object);
    auto shared = [&]() { return reinterpret_cast<Dword *>(object[9]); };
    if (!shared()[6]) {
        Dword *state = shared();
        const auto *dimensions = reinterpret_cast<const std::int16_t *>(image);
        state[9] = reinterpret_cast<Dword>(image);
        state[1] = static_cast<int>(dimensions[2]);
        state[2] = static_cast<int>(dimensions[3]);
        state[3] = (image[3] >> 28) + 1;
        state[4] = image_depth(image);
        state[5] = image_format(image);
        state[8] = 4294967295u;
        state[10] = 0;
        state[13] =
            reinterpret_cast<Dword>(eagl::allocator(state[3] * 4, "TARPrivate::mShared.mpPitch"));
        state[10] = reinterpret_cast<Dword>(find_palette(image));
        if (state[10]) {
            state[8] = palette_sequence++;
            auto *palette = reinterpret_cast<Dword *>(state[10]);
            auto *colours = reinterpret_cast<std::uint8_t *>(palette) +
                            ((palette[3] & 4096) ? palette[4] : 16);
            state[10] = reinterpret_cast<Dword>(colours);
            std::uint8_t entries[1024];
            for (unsigned i = 0; i != 256; ++i) {
                entries[i * 4] = colours[i * 4 + 2];
                entries[i * 4 + 1] = colours[i * 4 + 1];
                entries[i * 4 + 2] = colours[i * 4];
                entries[i * 4 + 3] = colours[i * 4 + 3];
            }
            backend::com(backend_device, 264, state[8], entries);
        }
        state = shared();
        state[11] = reinterpret_cast<Dword>(reinterpret_cast<std::uint8_t *>(image) +
                                           ((image[3] & 4096) ? image[4] : 16));
        backend::com(backend_device, 80, state[1], state[2], state[3], Dword(0), state[5], Dword(1),
                     state + 6);
    }
    upload(object);
    return object;
}
} // namespace nfs::graphics::tar
