#include "car_initialization_runtime.hpp"
#include "../../physics/bodies/shape_body_runtime.hpp"
#include "../../physics/vehicles/car_construction_runtime.hpp"
#include "../../ai/population/ai_population_runtime.hpp"
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include <cstring>
namespace nfs::game::world::vehicle_setup {

void reset_engine_limit(void *car) {
    int limit = at<int>(car, 660);
    at<int>(car, 28) = limit;
    if (limit > 11999)
        limit = 12000;
    at<int>(car, 28) = limit;
    runtime::physics::contact::reset_transmission(car);
}

void reset_wheel_contacts(void *car) {
    at<Dword>(car, 1792) &= 4294965247u;
    at<Byte>(car, 1692) = at<Byte>(car, 1693) = 0;
    for (int wheel = 0; wheel < 4; ++wheel) {
        runtime::particles::lifecycle::release_wheel_skid(car, wheel);
        at<Dword>(car, 1700 + wheel * 4) = 0;
        runtime::physics::contact::reset_wheel_state(bytes(car) + 1948 + wheel * 296);
    }
}
} // namespace nfs::game::world::vehicle_setup
namespace nfs::game::world::pending {

void reset_car_start_position(void *car) {
    using namespace runtime;
    using namespace vehicle_setup;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    float bounds_minimum[3], bounds_maximum[3];
    void *renderable = at<void *>(car, 3372);
    if (!renderable)
        bounds_maximum[0] = bounds_maximum[1] = bounds_maximum[2] = 1;
    else {
        mark_bounds_dirty(renderable);
        query_render_bounds(renderable, bounds_minimum, bounds_maximum);
    }
    physics::initialize_body_mass(at<void *>(car, 1768), at<float>(car, 520), bounds_maximum[0],
                                  bounds_maximum[1], bounds_maximum[2], 0, 15);
    configure_body_shape(car);
    adjust_body_for_tires(car);
    void *body = at<void *>(car, 1768);
    at<Byte>(body, 5) = 0;
    at<void *>(body, 0) = car;
    const Dword participant = at<Dword>(car, 1772);
    void *record = participant < 24 ? bytes(subsystem(72)) + 248 + participant * 84 : nullptr;
    const Dword slot = at<Dword>(record, 20);
    float position[3], rotation[9];
    std::memcpy(position, bytes(subsystem(68)) + 700 + slot * 12, 12);
    std::memcpy(rotation, bytes(subsystem(68)) + 988 + slot * 36, 36);
    alignas(4) Byte ground[84];
    // Constructor writes seven triplets, kind, and three shorts only.
    for (unsigned offset : {0u, 12u, 28u, 40u, 52u, 64u})
        std::memcpy(ground + offset, saved::zero_vector, 12);
    at<Dword>(ground, 24) = 0;
    at<std::uint16_t>(ground, 76) = at<std::uint16_t>(ground, 78) = at<std::uint16_t>(ground, 80) =
        0;
    physics::clear_ground_contact(ground);
    physics::surface_height(ground, position);
    position[1] = float(E(position[1]) + bounds_maximum[1]);
    if (at<Dword>(ground, 24)) {
        std::memcpy(rotation + 3, ground + 12, 12);
        cross_vectors_entry(reinterpret_cast<float *>(ground + 12), rotation + 6, rotation);
        matrices::orthonormalize(rotation);
        std::memcpy(rotation + 3, ground + 12, 12);
        matrices::orthonormalize(rotation);
    }
    body = at<void *>(car, 1768);
    std::memcpy(bytes(body) + 680, position, 12);
    std::memcpy(bytes(body) + 692, rotation, 36);
    matrices::transpose(&at<float>(body, 692), &at<float>(body, 72));
    physics::reset_body_ground(at<void *>(car, 1768), 16);
    body = at<void *>(car, 1768);
    float track_position[] = {at<float>(body, 680), at<float>(body, 684),
                              -at<float>(body, 688)};
    at<int>(car, 7984) =
        ai::lifecycle::pending::nearest_track_point(track_collision, track_position);
    for (unsigned offset :
         {3136u, 3140u, 3144u, 3172u, 3176u, 3180u, 3148u, 3152u, 3156u, 3160u, 3164u,
          3168u, 3196u, 3200u, 3204u, 3184u, 3188u, 3192u, 1800u})
        at<Dword>(car, offset) = 0;
    std::memcpy(bytes(car) + 1804, saved::zero_vector, 12);
    for (unsigned offset :
         {1592u, 1593u, 1594u, 1576u, 1581u, 1582u, 1583u, 1580u, 1584u, 1585u})
        at<Byte>(car, offset) = 0;
    at<Dword>(car, 1588) &= 4294967294u;
    at<Byte>(car, 1577) = at<Byte>(car, 1578) = 0;
    for (unsigned offset : {1596u, 1600u, 1604u})
        at<Dword>(car, offset) = 0;
    at<Byte>(car, 1586) = 0;
    at<Dword>(car, 1608) = at<Dword>(car, 1612) = 0;
    at<Byte>(car, 1579) = 0;
    at<Dword>(car, 1616) = 0;
    reset_engine_limit(car);
    at<Dword>(car, 372) = at<Dword>(car, 368) = at<Dword>(car, 364) = 0;
    std::memcpy(bytes(car) + 376, saved::identity_matrix, 36);
    std::memcpy(bytes(car) + 412, saved::identity_matrix, 36);
    for (unsigned offset : {448u, 452u, 460u, 456u, 468u, 472u, 464u})
        at<Dword>(car, offset) = 0;
    physics::construction::clear_car_run_statistics(bytes(car) + 1820);
    reset_wheel_contacts(car);
    physics::contact::reset_controls(car);
    for (unsigned offset = 1620; offset <= 1688; offset += 4)
        at<Dword>(car, offset) = 0;
    kernel::memory_zero(bytes(car) + 1700, 16);
    kernel::memory_zero(bytes(car) + 1695, 4);
    kernel::memory_zero(bytes(car) + 1720, 48);
    kernel::memory_zero(bytes(car) + 1716, 4);
    kernel::memory_zero(bytes(car) + 1692, 3);
    at<Dword>(car, 3208) = 0;
    kernel::memory_zero(bytes(car) + 3212, 160);
    at<Dword>(car, 1796) = at<Dword>(car, 1792) = 0;
    ai::lifecycle::reset_chase_contact(bytes(car) + 7952);
    at<Dword>(car, 7996) = at<Dword>(car, 8000) = 0;
}
} // namespace nfs::game::world::pending
