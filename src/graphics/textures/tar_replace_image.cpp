#include "nfs/graphics/resources/eagl_assets.hpp"
namespace nfs::graphics::tar {
namespace {
Dword *fallback() {
    return reinterpret_cast<Dword *>(fallback_image + reinterpret_cast<Dword *>(fallback_image)[5]);
}
} // namespace

std::uint8_t replace_image(Dword *object, Dword *image) {
    if (!image)
        image = fallback();
    const Dword format = image_format(image);
    Dword *caps = backend::adapter_caps();
    int index = 0;
    for (; index < 36; ++index)
        if (caps[6912 / 4 + index] == format)
            break;
    if (index == 36)
        image = fallback();
    auto *shared = reinterpret_cast<Dword *>(object[9]);
    --shared[0];
    if (!shared[0]) {
        if (backend::unresolved::texture_values[object[0]] == shared[6]) {
            backend::com(backend_device, 244, object[0], Dword(0));
            backend::unresolved::texture_values[object[0]] = 0;
        }
        shared_destroy(shared);
        shared = reinterpret_cast<Dword *>(object[9]);
        if (shared[12] & 2) {
            if (shared) {
                shared_destroy(shared);
                eagl::deallocator(shared, 56);
            }
            object[9] = 0;
        }
    }
    shared = private_data(image);
    object[9] = reinterpret_cast<Dword>(shared);
    if (!shared) {
        shared = static_cast<Dword *>(eagl::allocator(56, "EAGLInternal::SharedData new"));
        if (shared)
            shared_construct(shared);
        object[9] = reinterpret_cast<Dword>(shared);
        shared[12] |= 2;
    }
    if (!shared[6]) {
        shared[9] = reinterpret_cast<Dword>(image);
        shared[1] = static_cast<std::int16_t>(image[1]);
        shared[2] = static_cast<std::int16_t>(image[1] >> 16);
        shared[3] = (image[3] >> 28) + 1;
        shared[4] = image_depth(image);
        shared[5] = image_format(image);
        shared[8] = ~Dword(0);
        shared[13] =
            reinterpret_cast<Dword>(eagl::allocator(shared[3] * 4, "TARPrivate::mShared.mpPitch"));
        shared[10] = reinterpret_cast<Dword>(find_palette(image));
        if (shared[10]) {
            shared[8] = palette_sequence++;
            auto *palette = reinterpret_cast<Dword *>(shared[10]);
            shared[10] = (palette[3] & 4096) ? shared[10] + palette[4]
                                               : reinterpret_cast<Dword>(palette + 4);
            const auto *colours = reinterpret_cast<const std::uint8_t *>(shared[10]);
            std::uint8_t entries[1024];
            for (unsigned i = 0; i < 256; ++i) {
                entries[4 * i] = colours[4 * i + 2];
                entries[4 * i + 1] = colours[4 * i + 1];
                entries[4 * i + 2] = colours[4 * i];
                entries[4 * i + 3] = colours[4 * i + 3];
            }
            backend::com(backend_device, 264, shared[8], entries);
        }
        shared = reinterpret_cast<Dword *>(object[9]);
        shared[11] = (image[3] & 4096) ? reinterpret_cast<Dword>(image) + image[4]
                                         : reinterpret_cast<Dword>(image + 4);
        int result = backend::com(backend_device, 80, shared[1], shared[2], shared[3], Dword(0),
                                  shared[5], Dword(1), shared + 6);
        if (result < 0)
            return 0;
        upload(object);
    }
    ++reinterpret_cast<Dword *>(object[9])[0];
    return 1;
}
} // namespace nfs::graphics::tar
