#pragma once
#include "nfs/network/vehicles/network_snapshots.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/core/timing/frame_clock.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstddef>
#include <cstring>
namespace nfs::network::snapshots {

enum class PositionState : std::int16_t {
    Null = 0,
    Warp = 1,
    Interpolate = 2,
    Guide = 3,
    Drone = 4,
    Simulate = 5,
    Catchup = 6
};
struct Snapshot {
    Dword tick;
    float position[3];
    float rotation[9];
    float velocity[3];
    float angular_velocity[3];
    Dword flags;
};
struct SnapshotNode {
    Snapshot state;
    SnapshotNode *next;
    Dword active, position_only;
};
struct SnapshotQueue {
    CRITICAL_SECTION critical;
    Snapshot current;
    Dword position_only;
    float speed, velocity[3], prediction_error[3];
    SnapshotNode nodes[48];
    volatile Dword latest_tick;
    SnapshotNode *head, *tail;
};
static_assert(sizeof(Snapshot) == 80 && sizeof(SnapshotNode) == 92);
static_assert(offsetof(Snapshot, velocity) == 52 && offsetof(Snapshot, flags) == 76);
static_assert(offsetof(SnapshotQueue, current) == 24 && offsetof(SnapshotQueue, nodes) == 136);
static_assert(offsetof(SnapshotQueue, latest_tick) == 4552 &&
              offsetof(SnapshotQueue, head) == 4556);
static_assert(sizeof(SnapshotQueue) == 4564);
inline SnapshotQueue *car_queue(void *car) {
    return reinterpret_cast<SnapshotQueue *>(static_cast<std::uint8_t *>(car) + 3376);
}
inline void copy_state(Snapshot *destination, const Snapshot *source) {
    auto *out = reinterpret_cast<Dword *>(destination);
    const auto *in = reinterpret_cast<const Dword *>(source);
    for (unsigned word = 0; word < 20; ++word)
        out[word] = in[word];
}
void reset_queue(void *);
void destroy_queue(void *);
void peek(void *, void *);
Dword newest_tick(void *);
void apply_reset_snapshot(Dword, void *);
int camera_warp_requires_track_load(void *, const Snapshot *);
} // namespace nfs::network::snapshots
namespace nfs::game::world::camera {
void refresh_target(void *);
}
namespace nfs::game::runtime::track {
int following_pair(Dword *, int);
void select_network_section(Dword *, int, int);
} // namespace nfs::game::runtime::track
namespace nfs::game::runtime::physics {
Dword request_car_reset(void *, float);
void sample_track_reset_pose(void *, int, float *, float *);
} // namespace nfs::game::runtime::physics
namespace nfs::game::runtime::ai::lifecycle::pending {
void *section_position(void *, int);
Dword project_reset_position(void *, const float *, void **, int *, float *, float *, Dword, void *);
} // namespace nfs::game::runtime::ai::lifecycle::pending
