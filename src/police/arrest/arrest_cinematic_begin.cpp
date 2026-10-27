#include "arrest_cinematic_runtime.hpp"
#include "nfs/graphics/math/eagl_math.hpp"
#include "../../race/rules/race_rules_x87.hpp"
namespace nfs::game::world::actor_resources {
namespace {
void marker_facing(const float *quaternion, float *direction) {
    auto E = [](float value) { return static_cast<long double>(value); };
    const auto x2 = E(quaternion[0]) + quaternion[0], y2 = E(quaternion[1]) + quaternion[1];
    const float z2 = float(E(quaternion[2]) + quaternion[2]);
    const float xw = float(x2 * quaternion[3]), yw = float(E(quaternion[3]) * y2),
                zw = float(E(z2) * quaternion[3]);
    const float xx = float(x2 * quaternion[0]), xy = float(E(quaternion[0]) * y2),
                xz = float(E(z2) * quaternion[0]);
    const auto yy = y2 * quaternion[1], yz = E(z2) * quaternion[1], zz = E(z2) * quaternion[2];
    // Build all sixteen local words before transforming the forward vector.
    const float matrix[16] = {float(E(1.f) - (zz + yy)),
                              float(E(xy) + zw),
                              float(E(xz) - yw),
                              0.f,
                              float(E(xy) - zw),
                              float(E(1.f) - (zz + xx)),
                              float(yz + xw),
                              0.f,
                              float(E(xz) + yw),
                              float(yz - xw),
                              float(E(1.f) - (yy + xx)),
                              0.f,
                              0.f,
                              0.f,
                              0.f,
                              1.f};
    const float forward[3]{0, 0, 1};
    graphics::math::transform_position(forward, matrix, direction);
    direction[0] = -direction[0];
    direction[2] = -direction[2];
}
} // namespace

void begin_arrest_cinematic(void *owner, int marker_index, Dword callback, void *context) {
    const float *marker = static_cast<const float *>(
        runtime::level_marker(static_cast<Dword *>(subsystem(24)), 2, marker_index));
    float position[3], facing[3];
    std::memcpy(position, marker, 12);
    marker_facing(marker + 3, facing);
    position[2] = -position[2];
    unload_vehicle_drivers(subsystem(52));
    if (void *list = at<void *>(owner, 92))
        clear_actor_list(list);
    if (void *table = at<void *>(owner, 108)) {
        invoke(table, 0, 1);
        at<void *>(owner, 108) = nullptr;
    }
    release_actor_list_base();
    void *table =
        lifecycle::allocate(20, "CharacterSystem - charTable", lifecycle::character_pool);
    if (table) {
        try {
            construct_actor_list(table);
        } catch (...) {
            kernel::memory_free(table);
            throw;
        }
        at<const Dword *>(table, 0) = world::pending::character_table_methods;
    }
    at<void *>(owner, 108) = table;
    load_arrest_actors(owner);
    at<Dword>(owner, 96) = callback;
    at<void *>(owner, 100) = context;
    for (int index = 0;; ++index) {
        void *list = at<void *>(owner, 92);
        if (index >= (list ? at<int>(list, 12) : 0))
            break;
        if (!at<Byte **>(owner, 88)[at<Dword>(owner, 80)][index])
            continue;
        void *actor = list ? actor_at(list, index) : nullptr;
        std::memcpy(offset(actor, 124), position, 12);
        std::memcpy(offset(actor, 136), facing, 12);
        reset_animation_clock(offset(actor, 52));
        update_actor_pose(actor);
    }
    camera::begin_arrest_camera(tick_services[0]);
    at<Byte>(owner, 104) = 1;
    frame_clock::elapsed =
        float(static_cast<long double>(int(graphics::unresolved::camera_state)) *
              0.01666666753590106964111328125f);
}
} // namespace nfs::game::world::actor_resources
