#include "movie_runtime.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#if defined(_MSC_VER) && !defined(__MINGW32__)
using WINBOOL = BOOL; // Wine D3D8 declarations use the Win32 four-byte BOOL spelling.
#endif
#include "../../third_party/d3d8/d3d8types.h"
#include "../../third_party/d3d8/d3d8caps.h"
#include <cstring>
#include <new>
#include <stdexcept>

static_assert(sizeof(D3DCAPS8) == 212 && sizeof(D3DDEVICE_CREATION_PARAMETERS) == 16 &&
              sizeof(D3DDISPLAYMODE) == 16, "D3D8 Win32 device query ABI");

namespace nfs::movies {
namespace {
struct MovieTextureOwner {
    MovieTextureOwner *next;
    Dword *texture;
    Dword *device;
    bool discard_upload;
};
MovieTextureOwner *movie_textures = nullptr;

MovieTextureOwner *texture_owner(Dword *texture) {
    for (auto *owner = movie_textures; owner; owner = owner->next)
        if (owner->texture == texture)
            return owner;
    return nullptr;
}

[[noreturn]] void upload_error(const char *operation, int) {
    throw std::runtime_error(operation);
}

void release_gpu_texture(MovieTextureOwner *owner) {
    auto *shared = reinterpret_cast<Dword *>(owner->texture[9]);
    if (owner->device && shared[6]) {
        const Dword stage = owner->texture[0];
        Dword *bound = nullptr;
        // GetTexture returns a reference. Do not unbind an unrelated texture
        // merely because an old EAGL cache happens to contain our handle.
        const int queried = graphics::backend::com(owner->device, 240, stage, &bound);
        if (queried >= 0 && bound == reinterpret_cast<Dword *>(shared[6]))
            graphics::backend::com(owner->device, 244, stage, Dword(0));
        if (queried >= 0 && bound)
            graphics::backend::com(bound, 8);
        if (owner->device == graphics::backend_device &&
            graphics::backend::unresolved::texture_values[stage] == shared[6])
            graphics::backend::unresolved::texture_values[stage] = 0;
        graphics::backend::com(reinterpret_cast<Dword *>(shared[6]), 8);
        shared[6] = 0;
    }
    if (owner->device) {
        graphics::backend::com(owner->device, 8);
        owner->device = nullptr;
    }
}

bool supports_discard_upload(Dword *device, Dword format) {
    namespace backend = graphics::backend;
    D3DCAPS8 caps{};
    int result = backend::com(device, 28, &caps);
    if (result < 0)
        upload_error("MAD GetDeviceCaps", result);
    if (!(caps.Caps2 & D3DCAPS2_DYNAMICTEXTURES))
        return false;
    D3DDEVICE_CREATION_PARAMETERS creation{};
    result = backend::com(device, 36, &creation);
    if (result < 0)
        upload_error("MAD GetCreationParameters", result);
    D3DDISPLAYMODE mode{};
    result = backend::com(device, 32, &mode);
    if (result < 0)
        upload_error("MAD GetDisplayMode", result);
    Dword *direct3d = nullptr;
    result = backend::com(device, 24, &direct3d);
    if (result < 0 || !direct3d)
        upload_error("MAD GetDirect3D", result);
    result = backend::com(direct3d, 40, creation.AdapterOrdinal, creation.DeviceType,
                           mode.Format, Dword(D3DUSAGE_DYNAMIC), Dword(D3DRTYPE_TEXTURE), format);
    backend::com(direct3d, 8);
    return result >= 0;
}
}

void register_movie_texture(Dword *texture) {
    auto *owner = static_cast<MovieTextureOwner *>(
        eagl::allocator(sizeof(MovieTextureOwner), "Movie texture owner"));
    if (!owner)
        throw std::bad_alloc();
    owner->texture = texture;
    owner->device = nullptr;
    owner->discard_upload = false;
    owner->next = movie_textures;
    movie_textures = owner;
}

void destroy_movie_texture(Dword *texture) {
    MovieTextureOwner **link = &movie_textures;
    while (*link && (*link)->texture != texture)
        link = &(*link)->next;
    if (*link) {
        auto *owner = *link;
        release_gpu_texture(owner);
        *link = owner->next;
        eagl::deallocator(owner, sizeof(MovieTextureOwner));
    }
    graphics::tar::destroy(texture);
    eagl::deallocator(texture, 48);
}

void release_movie_device_textures() {
    for (auto *owner = movie_textures; owner; owner = owner->next)
        if (owner->device == graphics::backend_device)
            release_gpu_texture(owner);
}

bool movie_texture_needs_upload(Dword *texture) {
    auto *owner = texture_owner(texture);
    return owner && (owner->device != graphics::backend_device ||
                     !reinterpret_cast<Dword *>(texture[9])[6]);
}

// Authorized movie-only compatibility path. The ordinary TAR replacement and
// reference-counting contract is untouched: this renderer owns one private TAR.
void upload_frame(Image *image, Dword *texture) {
    namespace backend = graphics::backend;
    Dword *shape = image->shape;
    const int width = field<std::int16_t>(shape, 4);
    const int height = field<std::int16_t>(shape, 6);
    const Dword format = graphics::tar::image_format(shape);
    const int pixel_bytes = image->bytes_per_pixel;
    if (width <= 0 || height <= 0 ||
        !((pixel_bytes == 2 && (format == 25 || format == 23)) ||
          (pixel_bytes == 4 && format == 21)))
        upload_error("MAD unsupported frame layout", int(2289436780u));
    Dword *shared = reinterpret_cast<Dword *>(texture[9]);
    auto *owner = texture_owner(texture);
    if (!shared || shared[0] != 1 || !graphics::backend_device || !owner)
        upload_error("MAD private texture ownership", int(2289436780u));
    if (owner->device && owner->device != graphics::backend_device)
        release_gpu_texture(owner);
    const auto *pixels = reinterpret_cast<const Byte *>(shape) +
                         ((shape[3] & 4096) ? shape[4] : 16);
    const Dword row_bytes = Dword(width) * Dword(pixel_bytes);
    if (!shared[6]) {
        if (!owner->device) {
            owner->device = graphics::backend_device;
            backend::com(owner->device, 4);
            owner->discard_upload = supports_discard_upload(owner->device, format);
        }
        const int result = backend::com(owner->device, 80, Dword(width), Dword(height), Dword(1),
                                         Dword(owner->discard_upload ? D3DUSAGE_DYNAMIC : 0),
                                         format, Dword(owner->discard_upload ? D3DPOOL_DEFAULT
                                                                           : D3DPOOL_MANAGED),
                                         shared + 6);
        if (result < 0 || !shared[6])
            upload_error("MAD CreateTexture", result);
        shared[1] = Dword(width);
        shared[2] = Dword(height);
        shared[3] = 1;
        shared[4] = graphics::tar::image_depth(shape);
        shared[5] = format;
        shared[8] = ~Dword(0);
        shared[9] = reinterpret_cast<Dword>(shape);
        shared[10] = 0;
        shared[11] = reinterpret_cast<Dword>(pixels);
        if (!shared[13])
            shared[13] = reinterpret_cast<Dword>(eagl::allocator(4, "Movie texture pitch"));
        if (!shared[13])
            upload_error("MAD pitch allocation", int(2147942414u));
    } else if (shared[1] != Dword(width) || shared[2] != Dword(height) || shared[5] != format) {
        upload_error("MAD texture dimensions changed", int(2289436780u));
    }

    const Dword stage = texture[0];
    if (stage >= 8)
        upload_error("MAD texture stage", int(2289436780u));
    if (backend::unresolved::texture_values[stage] == shared[6]) {
        const int result = backend::com(graphics::backend_device, 244, stage, Dword(0));
        if (result < 0)
            upload_error("MAD unbind texture", result);
        backend::unresolved::texture_values[stage] = 0;
    }
    struct LockedRectangle { int pitch; Byte *pixels; } locked{};
    Dword *resource = reinterpret_cast<Dword *>(shared[6]);
    const int result = backend::com(resource, 64, Dword(0), &locked,
                                    static_cast<const RECT *>(nullptr),
                                    Dword(owner->discard_upload ? D3DLOCK_DISCARD : 0));
    if (result < 0)
        upload_error("MAD LockRect", result);
    if (!locked.pixels || locked.pitch < int(row_bytes)) {
        backend::com(resource, 68, Dword(0));
        upload_error("MAD invalid locked pitch", int(2289436780u));
    }
    for (int row = 0; row < height; ++row)
        std::memcpy(locked.pixels + std::size_t(row) * locked.pitch,
                    pixels + std::size_t(row) * row_bytes, row_bytes);
    const int unlocked = backend::com(resource, 68, Dword(0));
    if (unlocked < 0)
        upload_error("MAD UnlockRect", unlocked);
    *reinterpret_cast<Dword *>(shared[13]) = Dword(locked.pitch);
}
}
