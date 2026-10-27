#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include "nfs/graphics/display/viewport_transform.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::services::pending {
using game::runtime::at;
// The colour argument is unused in this entry.
void create_reflection_target(void *object, Dword kind, Dword width, Dword height, Dword colour_depth,
                              float viewport_width, float viewport_height, Dword) {
    at<Dword>(object, 104) = kind;
    Dword *target = graphics::targets::create(graphics::device);
    at<Dword *>(object, 0) = target;
    Dword *viewport = graphics::targets::create_viewport(target);
    at<Dword *>(object, 16) = viewport;
    if (!graphics::startup::display_caps ||
        (reinterpret_cast<Byte *>(graphics::startup::display_caps)[760] & 1))
        return;
    Dword *colour = graphics::tar::create_colour_target(width, height, colour_depth);
    at<Dword *>(object, 8) = colour;
    Dword *depth =
        graphics::tar::create_depth_target(graphics::context, width, height, colour_depth);
    colour[1] = colour[2] = colour[3] = colour[4] = 3;
    at<Dword *>(object, 12) = depth;
    graphics::targets::bind_textures(target, colour, depth);
    graphics::viewports::set_color(viewport, 4278190080u);

    if (!kind) {
        graphics::viewports::rectangle(viewport, 0, 0, viewport_width, viewport_height, 0, 1);
        graphics::viewports::orthographic(viewport, 1, 1, 21300.0f);
        graphics::viewports::pixel_orthographic(viewport, 1, 15000.0f);
    } else if (kind == 1) {
        graphics::viewports::rectangle(viewport, 0, 0, viewport_width, viewport_height, 0, 1);
        graphics::viewports::perspective(viewport, 60, 1.33333337306976318359375f, 1, 50000);
    }
    at<Byte>(object, 100) = 1;
}

void release_reflection_target(void *object) {
    Dword *viewport = at<Dword *>(object, 16);
    at<Byte>(object, 100) = 0;
    if (viewport)
        if (Dword *target = at<Dword *>(object, 0))
            graphics::targets::viewport_destroy(target, viewport);
    Dword *target = at<Dword *>(object, 0);
    at<Dword *>(object, 16) = nullptr;
    if (target)
        graphics::targets::destroy(graphics::device, target);
    void *data = at<void *>(object, 4);
    at<Dword *>(object, 0) = nullptr;
    if (data)
        kernel::memory_free(data);
    Dword *colour = at<Dword *>(object, 8);
    at<void *>(object, 4) = nullptr;
    if (colour) {
        graphics::tar::destroy(colour);
        eagl::deallocator(colour, 48);
    }
    Dword *depth = at<Dword *>(object, 12);
    at<Dword *>(object, 8) = nullptr;
    if (depth) {
        graphics::tar::destroy(depth);
        eagl::deallocator(depth, 48);
    }
    at<Dword *>(object, 12) = nullptr;
}
} // namespace nfs::services::pending
