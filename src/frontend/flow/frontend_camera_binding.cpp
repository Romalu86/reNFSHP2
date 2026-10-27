#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
namespace nfs::game::world::pending {

Dword frontend_camera_viewport[9] = {0, 0,          1145569280, 1142292480, 0,
                                    0, 1145569280, 1142292480, 1065353216};

void copy_frontend_camera_viewport(void *camera) {
    if (graphics::device) {
        float width, height;
        graphics::backend::get_dimensions(graphics::context, &width, &height);
        frontend_camera_viewport[6] = frontend_camera_viewport[2] =
            *reinterpret_cast<Dword *>(&width);
        frontend_camera_viewport[3] = frontend_camera_viewport[7] =
            *reinterpret_cast<Dword *>(&height);
        for (int index = 0; index != 9; ++index)
            at<Dword>(camera, 124 + index * 4) = frontend_camera_viewport[index];
    }
}

void bind_frontend_camera(void *camera) {
    if (camera) {
        if (auto *pose = runtime::invoke<Dword *>(camera, 2)) {
            runtime::race_audio::camera_orientation(
                pose, reinterpret_cast<float *>(offset(camera, 32)));
            const Dword x = at<Dword>(pose, 4), y = at<Dword>(pose, 8), z = at<Dword>(pose, 12);
            at<Dword>(camera, 20) = x;
            at<Dword>(camera, 24) = y;
            at<Dword>(camera, 116) = at<Dword>(pose, 28);
            at<Dword>(camera, 28) = z;
        }
    }
    auto *matrix = reinterpret_cast<float *>(offset(camera, 80));
    runtime::matrices::transpose(reinterpret_cast<float *>(offset(camera, 32)), matrix);
    const float negative_position[3] = {-at<float>(camera, 20), -at<float>(camera, 24),
                                        -at<float>(camera, 28)};
    float translation[3];
    runtime::matrices::transform_vectors(1, negative_position, matrix, translation);
    for (int index = 0; index != 3; ++index)
        at<float>(camera, 68 + index * 4) = translation[index];
    copy_frontend_camera_viewport(camera);
}
} // namespace nfs::game::world::pending
