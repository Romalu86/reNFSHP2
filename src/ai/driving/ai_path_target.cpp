#include "ai_path_frame_x87.hpp"
#include "ai_route_planner.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include <cmath>
namespace nfs::game::runtime::ai::entities::pending {
using kernel::numeric::Extended;
float minimum_path_width =
    10.f;

Extended horizontal_distance_squared(const float *first, const float *second) {
    alignas(4) unsigned char retained[10];
    path_frame::horizontal_distance_native80(first, second, retained);
    return Extended::from_native80(retained, kernel::numeric::current_format());
}

Extended advance_path_distance(void *path, float distance, int *offset, Dword *point) {
    alignas(4) unsigned char retained[10];
    path_frame::advance_distance_native80(path, distance, offset, point, retained);
    return Extended::from_native80(retained, kernel::numeric::current_format());
}

void reverse_entity_path(void *entity) {
    void *path = bytes(entity) + 220;
    lifecycle::reverse_path(path);
    update_path_sample(path, invoke<const float *>(entity, 0));
    lifecycle::pending::update_path_branch(path, entity);
    int offset;
    Dword point;
    path_frame::discard_advance_distance(path, 20.f, &offset, &point);
}

Dword sample_path_target(void *path, float *output, float *width, Dword *point, float *pose,
                        float distance, const float *position, Byte reverse) {
    return path_frame::sample_target(path, output, width, point, pose, distance, position, reverse);
}
} // namespace nfs::game::runtime::ai::entities::pending
