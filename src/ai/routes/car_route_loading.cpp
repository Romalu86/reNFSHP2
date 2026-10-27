#include "../../physics/bodies/shape_body_runtime.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "../../race/rules/race_rules_x87.hpp"
namespace nfs::game::world {

Byte ground_covers_spawn(const float *position, float distance) {
    alignas(4) Byte contact[84];
    runtime::physics::pending::construct_ground_contact(contact);
    runtime::physics::clear_ground_contact(contact);
    float sample[] = {float(static_cast<long double>(position[0]) + distance), position[1],
                      position[2]};
    if (!runtime::physics::sample_ground(sample, contact))
        return 0;
    sample[2] = float(static_cast<long double>(sample[2]) + distance);
    if (!runtime::physics::sample_ground(sample, contact))
        return 0;
    sample[0] = float(static_cast<long double>(sample[0]) - distance);
    if (!runtime::physics::sample_ground(sample, contact))
        return 0;
    sample[0] = float(static_cast<long double>(sample[0]) - distance);
    if (!runtime::physics::sample_ground(sample, contact))
        return 0;
    sample[2] = float(static_cast<long double>(sample[2]) - distance);
    if (!runtime::physics::sample_ground(sample, contact))
        return 0;
    sample[2] = float(static_cast<long double>(sample[2]) - distance);
    if (!runtime::physics::sample_ground(sample, contact))
        return 0;
    sample[0] = float(static_cast<long double>(sample[0]) + distance);
    if (!runtime::physics::sample_ground(sample, contact))
        return 0;
    sample[0] = float(static_cast<long double>(sample[0]) + distance);
    return runtime::physics::sample_ground(sample, contact) != 0;
}

Dword find_supported_spawn(void *compartment, float *position) {
    Dword *polygons = at<Dword *>(compartment, 12);
    for (int index = 0; index < int(polygons[0]); ++index) {
        const float *corners = reinterpret_cast<const float *>(polygons[1] + Dword(index) * 64);
        float difference[3], half[3];
        for (unsigned axis = 0; axis < 3; ++axis)
            difference[axis] = float(static_cast<long double>(corners[axis]) - corners[axis + 6]);
        runtime::scale_vectors(1, difference, .5f, half);
        for (unsigned axis = 0; axis < 3; ++axis)
            position[axis] = float(static_cast<long double>(half[axis]) + corners[axis]);
        if (ground_covers_spawn(position, .5f) && ground_covers_spawn(position, 1.f) &&
            ground_covers_spawn(position, 2.f) && ground_covers_spawn(position, 3.f) &&
            ground_covers_spawn(position, 4.f))
            return 1;
    }
    return 0;
}
namespace pending {

void refresh_car_routes(void *owner) {
    audio::vehicle::Progress progress;
    audio::vehicle::construct_progress(&progress, 24);
    try {
        for (unsigned index = 0; index < 24; ++index) {
            audio::vehicle::advance_progress(&progress);
            if (at<Byte>(owner, 1852 + index)) {
                alignas(4) Byte contact[84];
                runtime::physics::pending::construct_ground_contact(contact);
                runtime::physics::clear_ground_contact(contact);
                float *position = reinterpret_cast<float *>(offset(owner, 700 + index * 12));
                if (!runtime::physics::sample_ground(position, contact))
                    if (void *compartment =
                            runtime::track::boundary(static_cast<Dword *>(subsystem(36)), 1, 0))
                        find_supported_spawn(compartment, position);
            }
        }
    } catch (...) {
        audio::vehicle::destroy_progress(&progress);
        throw;
    }
    audio::vehicle::destroy_progress(&progress);
}
} // namespace pending
} // namespace nfs::game::world
