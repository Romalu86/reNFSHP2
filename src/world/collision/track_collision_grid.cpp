#include "nfs/world/lifecycle/world_dependency.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../../physics/motion/world_motion_x87.hpp"
#include <algorithm>

namespace nfs::game::world::dependency {
// Four interleaved X/Z points; each conversion follows a float store before the FISTP rounding
// instruction.
void static_shape_grid_bounds(const void *shape, int *minimum_x, int *minimum_z, int *maximum_x,
                              int *maximum_z) {
    const auto *corners = static_cast<const float *>(shape);
    for (int corner = 0; corner != 4; ++corner) {
        int x = runtime::hardware::current_integer_f32(
            float(static_cast<long double>(corners[corner * 2]) * 0.02f + 32.0f));
        int z = runtime::hardware::current_integer_f32(
            float(static_cast<long double>(corners[corner * 2 + 1]) * 0.02f + 32.0f));
        x = (std::clamp)(x, 0, 63);
        z = (std::clamp)(z, 0, 63);
        if (!corner) {
            *maximum_x = x;
            *minimum_x = x;
            *maximum_z = z;
            *minimum_z = z;
        } else {
            *minimum_x = (std::min)(*minimum_x, x);
            *maximum_x = (std::max)(*maximum_x, x);
            *minimum_z = (std::min)(*minimum_z, z);
            *maximum_z = (std::max)(*maximum_z, z);
        }
    }
}
namespace pending {

void insert_grid_object(void *object) {
    float position[3];
    runtime::invoke(object, 0, position);
    int x = runtime::hardware::current_integer_f32(
        float(static_cast<long double>(position[0]) * 0.02f + 32.0f));
    int z = runtime::hardware::current_integer_f32(
        float(static_cast<long double>(position[2]) * 0.02f + 32.0f));
    x = (std::clamp)(x, 0, 63);
    z = (std::clamp)(z, 0, 63);
    if (Byte *destination = runtime::physics::cell(x, z)) {
        void *previous = at<void *>(destination, 16);
        at<void *>(object, 8) = previous;
        at<void *>(object, 12) = destination + 8;
        if (previous)
            at<void *>(previous, 12) = object;
        at<void *>(destination, 16) = object;
    }
}
} // namespace pending
} // namespace nfs::game::world::dependency

namespace nfs::game::world::pending {
// Counts, allocates and fills the shared static-shape index.
Byte initialize_track_collision_grid() {
    auto clear_cell = [](Byte *cell) {
        at<Dword>(cell, 0) = 0;
        at<Dword>(cell, 4) = 0;
        at<Dword>(cell, 12) = 0;
        at<Dword>(cell, 20) = 0;
        at<Dword>(cell, 16) = 0;
    };
    for (int index = 0; index != 64 * 64; ++index)
        clear_cell(runtime::physics::pending::spatial_storage + index * 24);
    clear_cell(runtime::physics::pending::global_collision_cell);
    clear_cell(runtime::physics::pending::spatial_storage + 98304);

    int reference_count = 0;
    for (int shape = 0; shape < at<int>(subsystem(24), 164); ++shape) {
        int minimum_x, minimum_z, maximum_x, maximum_z;
        dependency::static_shape_grid_bounds(dependency::static_track_shape(subsystem(24), shape),
                                             &minimum_x, &minimum_z, &maximum_x, &maximum_z);
        for (int x = minimum_x; x <= maximum_x; ++x) {
            reference_count += maximum_z - minimum_z + 1;
            for (int z = minimum_z; z <= maximum_z; ++z)
                ++at<Dword>(runtime::physics::cell(x, z), 0);
        }
    }
    dependency::static_references = static_cast<Dword *>(
        kernel::memory_allocate("StaticRefs", reference_count * 2, lifecycle::level_pool));
    auto cursor = reinterpret_cast<std::uint16_t *>(dependency::static_references);
    // The saved allocation traversal is Z-major although the cell array is X-major.
    for (int z = 0; z != 64; ++z) {
        for (int x = 0; x != 64; ++x) {
            Byte *cell = runtime::physics::cell(x, z);
            Dword count = at<Dword>(cell, 0);
            at<std::uint16_t *>(cell, 4) = cursor;
            at<Dword>(cell, 0) = 0;
            cursor = reinterpret_cast<std::uint16_t *>(reinterpret_cast<Dword>(cursor) + count * 2);
        }
    }
    for (int shape = 0; shape < at<int>(subsystem(24), 164); ++shape) {
        int minimum_x, minimum_z, maximum_x, maximum_z;
        dependency::static_shape_grid_bounds(dependency::static_track_shape(subsystem(24), shape),
                                             &minimum_x, &minimum_z, &maximum_x, &maximum_z);
        for (int x = minimum_x; x <= maximum_x; ++x) {
            for (int z = minimum_z; z <= maximum_z; ++z) {
                Byte *cell = runtime::physics::cell(x, z);
                Dword count = at<Dword>(cell, 0);
                at<std::uint16_t *>(cell, 4)[count] = static_cast<std::uint16_t>(shape);
                at<Dword>(cell, 0) = count + 1;
            }
        }
    }
    return 1;
}
} // namespace nfs::game::world::pending
