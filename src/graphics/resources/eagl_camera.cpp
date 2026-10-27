#include "nfs/graphics/resources/eagl_camera.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>

namespace nfs::graphics::cameras {
const gui::Destroy camera_methods[1] = {reinterpret_cast<gui::Destroy>(delete_camera)};
const gui::Destroy base_methods[1] = {reinterpret_cast<gui::Destroy>(delete_base)};


void look_at(Camera *camera, const float *eye, const float *target, const float *up) {
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    auto length = [&](const float *vector) {
        const auto xx = E(vector[0]) * vector[0];
        const auto yy = E(vector[1]) * vector[1];
        const auto zz = E(vector[2]) * vector[2];
        return ((xx + yy) + zz).square_root();
    };
    for (unsigned i = 0; i != 4; ++i)
        camera->eye[i] = eye[i];
    for (unsigned i = 0; i != 4; ++i)
        camera->target[i] = target[i];
    for (unsigned i = 0; i != 4; ++i)
        camera->up[i] = up[i];
    math::Matrix translation;
    math::translation(translation, -eye[0], -eye[1], -eye[2]);
    for (unsigned i = 0; i != 4; ++i)
        camera->backward[i] = float(E(eye[i]) - target[i]);
    float distance = float(length(camera->backward));
    camera->distance = distance;
    if (!(0.f < distance))
        _assert("d > 0", "../../../cmn/eaglutil/src/camera.cpp", 49);
    auto inverse = E(1) / E(distance);
    for (unsigned i = 0; i != 3; ++i)
        camera->backward[i] = float(inverse * E(camera->backward[i]));
    math::cross(up, camera->backward, camera->right);
    const auto right_length = length(camera->right);
    distance = float(right_length);
    if (!(E(0) < right_length))
        _assert("d>0", "../../../cmn/eaglutil/src/camera.cpp", 59);
    inverse = E(1) / E(distance);
    for (unsigned i = 0; i != 3; ++i)
        camera->right[i] = float(inverse * E(camera->right[i]));
    math::cross(camera->backward, camera->right, camera->vertical);
    camera->vertical[3] = camera->backward[3] = camera->right[3] = 1;
    const float vertical_inverse = float(E(1) / length(camera->vertical));
    for (unsigned i = 0; i != 3; ++i)
        camera->vertical[i] = float(E(vertical_inverse) * camera->vertical[i]);
    for (unsigned i = 0; i != 4; ++i)
        camera->up[i] = camera->vertical[i];
    math::Matrix rotation;
    for (unsigned row = 0; row != 3; ++row) {
        rotation[row * 4] = camera->right[row];
        rotation[row * 4 + 1] = camera->vertical[row];
        rotation[row * 4 + 2] = camera->backward[row];
        rotation[row * 4 + 3] = 0;
    }
    rotation[12] = rotation[13] = rotation[14] = 0;
    rotation[15] = 1;
    math::Matrix result;
    math::multiply(translation, rotation, result);
    for (unsigned i = 0; i != 16; ++i)
        camera->view[i] = result[i];
}


void destroy(Camera *camera) {
    camera->methods = base_methods;
}


void *__fastcall delete_camera(Camera *camera, void *, unsigned flags) {
    destroy(camera);
    if (flags & 1)
        eagl::deallocator(camera, 172);
    return camera;
}


void *__fastcall delete_base(Camera *camera, void *, unsigned flags) {
    camera->methods = base_methods;
    if (flags & 1)
        eagl::deallocator(camera, 72);
    return camera;
}
} // namespace nfs::graphics::cameras
