#include "race_frame_draw.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::game::world::frame {
char capture_name[19];

Dword read_keyboard_key(int key) {
    if (network::session_active() && (phase_reset || runtime::reset_complete || runtime::reset_requested))
        return 0;
    if (key < 0 || key >= 256 || !keyboard_snapshot)
        return 0;
    return at<Byte>(reinterpret_cast<void *>(keyboard_snapshot), Dword(key));
}
 
void begin_frame_capture(Dword *) {
}

void end_frame_capture(Dword *context) {
    if (!launch::field<Dword>(8627796) || !read_keyboard_key(25))
        return;
    std::strcpy(capture_name, "testimage_0000.tga");
    int index = 0;
    while (files::file::exists(capture_name) && index < 9999)
        std::sprintf(capture_name, "testimage_%04d.tga", ++index);
    capture_backbuffer(context, capture_name, 0);
}
struct SurfaceDescription {
    Dword format, type, usage, pool, bytes, multisample, width, height;
};
struct LockedSurface {
    int pitch;
    Byte *data;
};
static_assert(sizeof(SurfaceDescription) == 32 && sizeof(LockedSurface) == 8);

Byte capture_backbuffer(Dword *context, const char *filename, Byte opaque) {
    using graphics::backend::com;
    Dword *surface;
    if (com(graphics::backend_device, 64, Dword(0), Dword(0), &surface) < 0)
        return 0;
    SurfaceDescription description;
    com(surface, 32, &description);
    const int depth = at<int>(at<void *>(context, 4), 44);
    // Both saved low-depth exits leave the acquired surface untouched.
    if (depth <= 16)
        return 0;
    if (depth <= 24)
        return 0;
    const RECT rectangle = {0, 0, LONG(description.width), LONG(description.height)};
    LockedSurface locked;
    if (com(surface, 36, &locked, &rectangle, Dword(0)) < 0) {
        com(surface, 8);
        return 0;
    }
    Byte header[18]{};
    header[2] = 2;
    const std::uint16_t width = std::uint16_t(description.width), height = std::uint16_t(description.height);
    std::memcpy(header + 12, &width, 2);
    std::memcpy(header + 14, &height, 2);
    header[16] = Byte(depth);
    header[17] = 40;
    FILE *output = std::fopen(filename, "wb");
    if (!output) {
        com(surface, 40);
        com(surface, 8);
        return 0;
    }
    std::fwrite(header, 18, 1, output);
    if (opaque)
        for (Dword row = 0; row < description.height; ++row)
            for (Dword column = 0; column < description.width; ++column)
                at<Dword>(locked.data + row * Dword(locked.pitch), column * 4) |= 4278190080u;
    for (Dword row = 0; row < description.height; ++row)
        std::fwrite(locked.data + row * Dword(locked.pitch), 4, description.width, output);
    std::fclose(output);
    com(surface, 40);
    com(surface, 8);
    return 1;
}
} // namespace nfs::game::world::frame
