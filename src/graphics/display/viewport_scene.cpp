#include "nfs/graphics/display/viewport_transform.hpp"
#include "nfs/graphics/math/eagl_math.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include <intrin.h>
#include <cmath>
#include <cstring>

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Viewport scene numbers require native x87 intermediates");
namespace nfs::graphics::viewports {
std::uint8_t switching = 0;


Dword *target_viewport(const Dword *target) {
    return reinterpret_cast<Dword *>(target[36 / 4]);
}


void set_target_viewport(Dword **owner, Dword *viewport) {
    (*owner)[36 / 4] = reinterpret_cast<Dword>(viewport);
}

 
void begin(Dword *object) {
    switching = 1;
    if (object[9]) {
        if (current(reinterpret_cast<Dword *>(object[9])) &&
            reinterpret_cast<std::uint8_t *>(current(reinterpret_cast<Dword *>(object[9])))[360]) {
            Dword *previous = current(reinterpret_cast<Dword *>(object[9]));
            object[1] = reinterpret_cast<Dword>(previous);
            Dword suspended_parent = previous[1];
            end(previous);
            reinterpret_cast<Dword *>(object[1])[1] = suspended_parent;
        }
    } else {
        if (target_viewport(reinterpret_cast<Dword *>(object[10])) &&
            reinterpret_cast<std::uint8_t *>(target_viewport(reinterpret_cast<Dword *>(object[10])))[360]) {
            Dword *previous = target_viewport(reinterpret_cast<Dword *>(object[10]));
            object[1] = reinterpret_cast<Dword>(previous);
            end(previous);
        }
    }
    if (!object[1])
        object[1] = 1;
    Dword colour_target, depth_target;
    if (object[9]) {
        auto *context = reinterpret_cast<Dword *>(object[9]);
        select(reinterpret_cast<Dword **>(context + 2), object);
        context = reinterpret_cast<Dword *>(object[9]);
        colour_target = context[200 / 4];
        depth_target = context[204 / 4];
    } else {
        auto *target = reinterpret_cast<Dword *>(object[10]);
        set_target_viewport(reinterpret_cast<Dword **>(target + 1), object);
        target = reinterpret_cast<Dword *>(object[10]);
        colour_target = target[52 / 4];
        depth_target = target[56 / 4];
    }
    backend::com(backend_device, 124, colour_target, depth_target);
    backend::com(backend_device, 136);
    backend::com(backend_device, 160, object + 3);
    reinterpret_cast<std::uint8_t *>(object)[360] = 1;
    math::Matrix combined;
    math::copy(combined, reinterpret_cast<float *>(object + 116 / 4));
    math::append_projection(combined, reinterpret_cast<float *>(object + 52 / 4));
    __movsd(reinterpret_cast<unsigned long *>(object + 180 / 4),
            reinterpret_cast<const unsigned long *>(combined), 16);
    __movsd(reinterpret_cast<unsigned long *>(backend::view_matrix),
            reinterpret_cast<const unsigned long *>(object + 116 / 4), 16);
    __movsd(reinterpret_cast<unsigned long *>(backend::projection_matrix),
            reinterpret_cast<const unsigned long *>(object + 52 / 4), 16);
    __movsd(reinterpret_cast<unsigned long *>(backend::view_projection_matrix),
            reinterpret_cast<const unsigned long *>(combined), 16);
    const auto half_width = -(static_cast<long double>(std::uint64_t(object[5])) * 0.5f);
    const float half_height = float(-(static_cast<long double>(std::uint64_t(object[6])) * 0.5f));
    float *screen = reinterpret_cast<float *>(backend::screen_info);
    if (static_cast<long double>(screen[0]) != half_width || screen[1] != half_height) {
        screen[0] = float(half_width);
        const auto inverse_width = -1.0L / half_width;
        switching = 0;
        screen[1] = half_height;
        screen[2] = float(inverse_width);
        screen[3] = float(1.0L / static_cast<long double>(half_height));
        return;
    }
    switching = 0;
}


void end(Dword *object) {
    if (object[9])
        select(reinterpret_cast<Dword **>(reinterpret_cast<Dword *>(object[9]) + 2), nullptr);
    else
        set_target_viewport(reinterpret_cast<Dword **>(reinterpret_cast<Dword *>(object[10]) + 1),
                            nullptr);
    backend::com(backend_device, 140);
    reinterpret_cast<std::uint8_t *>(object)[360] = 0;
    if (!switching && object[1] && object[1] != 1)
        begin(reinterpret_cast<Dword *>(object[1]));
    object[1] = 0;
}


void clear(Dword *object, std::uint8_t flags) {
    Dword clear_flags = flags & 7;
    Dword *device = *static_cast<Dword *volatile *>(&backend_device);

    const Dword depth = object[356 / 4];
    backend::com(device, 144, Dword(0), Dword(0), clear_flags, object[48 / 4], depth,
                 Dword(0));
}
} // namespace nfs::graphics::viewports
