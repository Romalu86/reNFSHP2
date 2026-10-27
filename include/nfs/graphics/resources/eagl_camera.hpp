#pragma once
#include "nfs/graphics/math/eagl_math.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
namespace nfs::graphics::cameras {
struct Camera {
    const gui::Destroy *methods;
    math::Matrix view;
    kernel::Dword state;
    math::Vector eye, target, up;
    float distance;
    math::Vector right, vertical, backward;
};
static_assert(sizeof(Camera) == 172 && offsetof(Camera, eye) == 72 &&
              offsetof(Camera, backward) == 156);
extern const gui::Destroy camera_methods[1], base_methods[1];
void look_at(Camera *, const float *, const float *, const float *);
void destroy(Camera *);
void *__fastcall delete_camera(Camera *, void *, unsigned);
void *__fastcall delete_base(Camera *, void *, unsigned);
} // namespace nfs::graphics::cameras
