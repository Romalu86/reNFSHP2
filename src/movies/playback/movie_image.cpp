#include "movie_runtime.hpp"
#include "nfs/graphics/textures/shape_conversion.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include <cstring>
#include <stdexcept>
namespace nfs::movies {
const Dword image_methods[1] = {reinterpret_cast<Dword>(delete_image)};

int binary_exponent(int value) {
    float converted = float(integer(value));
    std::int32_t bits;
    std::memcpy(&bits, &converted, 4);
    return (bits >> 23) - 127;
}
// The fifth caller argument is not read.
Dword *allocate_image(int width, int height, int depth, Dword pool, const char *) {
    int x = binary_exponent(int(Dword(width) - 1));
    int y = binary_exponent(int(Dword(height) - 1));
    return graphics::textures::shape::allocate_shape(int(1u << ((Dword(x) + 1) & 31)),
                                                     int(1u << ((Dword(y) + 1) & 31)), depth, 0, 0,
                                                     pool | 1024, 0, 64);
}

Image *construct_image(Image *image, void *frame, graphics::Device *device, Dword *context,
                       Dword *immediate, int format) {
    image->device = device;
    image->methods = image_methods;
    image->context = context;
    image->immediate = immediate;
    image->format = format;
    void *shape = field<void *>(frame, 4);
    int width = field<std::int16_t>(shape, 4), height = field<std::int16_t>(shape, 6);
    float screen_width, screen_height;
    graphics::backend::get_dimensions(context, &screen_width, &screen_height);
    float stored_width = float(integer(width)), stored_height = float(integer(height));
    image->scale_x = image->scale_y = 1;
    image->z = 0;
    image->x = float((number(screen_width) - stored_width) * 0.5f);
    image->y = float((number(screen_height) - stored_height) * 0.5f);
    int depth;
    std::memcpy(&depth, &stored_width, 4); // Default retains the FSTP representation.
    switch (format) {
    case 1:
        image->bytes_per_pixel = 4;
        image->conversion = 1;
        depth = 32;
        break;
    case 2:
        image->bytes_per_pixel = 2;
        image->conversion = 2;
        depth = 15;
        break;
    case 3:
        image->bytes_per_pixel = 2;
        image->conversion = 3;
        depth = 16;
        break;
    case 4:
        image->bytes_per_pixel = 2;
        image->conversion = 0;
        depth = 16;
        break;
    }
    image->shape = allocate_image(width, height, depth, memory.pool, "YUV");
    if (format == 4)
        *reinterpret_cast<Byte *>(image->shape) = 104;
    image->height = stored_height;
    image->width = stored_width;
    return image;
}

void destroy_image(Image *image) {
    image->methods = image_methods;
    graphics::textures::shape::release_shape(image->shape);
    image->shape = nullptr;
}

void *__fastcall delete_image(Image *image, void *, unsigned flags) {
    destroy_image(image);
    if (flags & 1)
        memory.release(image);
    return image;
}

void copy_frame(Image *image, void *frame, Dword *texture) {
    void *input_shape = field<void *>(frame, 4);
    auto *input = static_cast<Byte *>(input_shape) +
                  ((field<Dword>(input_shape, 12) & 4096) ? field<Dword>(input_shape, 16) : 16);
    int width = field<std::int16_t>(input_shape, 4), height = field<std::int16_t>(input_shape, 6);
    auto *chroma = input + Dword(width) * Dword(height);
    auto *second_chroma = chroma + Dword(width / 2) * Dword(height / 2);
    auto *output = reinterpret_cast<Byte *>(image->shape) +
                   ((image->shape[3] & 4096) ? image->shape[4] : 16);
    Dword stride = Dword(field<std::int16_t>(image->shape, 4)) * Dword(image->bytes_per_pixel);
    const Dword result = pending::convert_yuv(input, chroma, second_chroma, Dword(width),
                                             Dword(width), height, output, stride,
                                             Dword(image->conversion));
    if (result) {
        throw std::runtime_error("MAD YUV conversion failed");
    }
    // Movie-only compatibility: retain the managed texture across all frames.
    upload_frame(image, texture);
}
} // namespace nfs::movies
