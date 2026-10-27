#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/graphics/skids/skid_lifecycle.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include "../../gui/lifecycle/gui_images.hpp"
#include "nfs/core/errors/assertions.hpp"
#include <cstring>
namespace nfs::game::runtime::particles::lifecycle {
void draw_particle_billboard(void *, const float *, const float *);

void destroy_render_item_interface(void *item) {
    at<const Dword *>(item, 0) = pending::render_item_base_methods;
}

void *__fastcall delete_render_item_interface(void *item, void *, Byte flags) {
    destroy_render_item_interface(item);
    if (flags & 1)
        gui::application_release(item);
    return item;
}

void *__fastcall delete_render_item_base(void *item, void *, Byte flags) {
    at<const Dword *>(item, 0) = pending::render_item_base_methods;
    if (flags & 1)
        gui::application_release(item);
    return item;
}
// RET4; argument is not read.
float __fastcall render_item_depth(void *item, void *, Dword) {
    return at<float>(at<void *>(item, 4), 0);
}

void __fastcall draw_render_item(void *item, void *) {
    using namespace graphics;
    void *renderer = at<void *>(nfs::lifecycle::runtime::particle_owner, 8);
    immediate::set_gouraud_state(bindings::single_draw, &at<Dword>(renderer, 12508));
    kernel::memory_copy(bindings::single_draw + 84 / 4, skids::lifecycle::identity_matrix, 64);
    immediate::set_render_state(bindings::single_draw_texture, &at<Dword>(renderer, 12576));
    kernel::memory_copy(bindings::single_draw_texture + 84 / 4, skids::lifecycle::identity_matrix,
                        64);
    void *range = at<void *>(item, 4);
    int index = at<int>(range, 12);
    const int end = at<int>(range, 16);
    while (index < end) {
        draw_particle_billboard(at<void *>(renderer, 8004 + Dword(index) * 4),
                                &at<float>(renderer, 12644), &at<float>(renderer, 12656));
        ++index;
    }
}
namespace {
} // namespace
const Dword pending::render_item_methods[3] = {
    reinterpret_cast<Dword>(delete_render_item_interface), reinterpret_cast<Dword>(draw_render_item),
    reinterpret_cast<Dword>(render_item_depth)};
const Dword pending::render_item_base_methods[3] = {
    reinterpret_cast<Dword>(delete_render_item_base), reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call),
    reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call)};
} // namespace nfs::game::runtime::particles::lifecycle
