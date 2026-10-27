#include "shape_body_runtime.hpp"
#include "../../ai/population/ai_population_runtime.hpp"
#include "../motion/world_motion_x87.hpp"
#include <cstring>
namespace nfs::game::runtime::physics {
// The final two padding bytes remain untouched.
void clear_ground_contact(void *contact) {
    for (unsigned offset = 0; offset <= 72; offset += 4)
        at<Dword>(contact, offset) = 0;
    at<std::uint16_t>(contact, 76) = 0;
    at<std::uint16_t>(contact, 78) = 0;
    at<std::uint16_t>(contact, 80) = 0;
}
// AL is the result consumed by the shape owner.
Byte initialize_shape_body(void *body, void *shape, Byte skip_ground) {
    float position[3], rotation[9];
    invoke(shape, 0, position);
    alignas(4) Byte ground[84];
    clear_ground_contact(ground);
    if (!skip_ground && !sample_ground(position, ground)) {
        release_vehicle(body);
        return 0;
    }
    at<Byte>(body, 728) = 1;
    at<Byte>(body, 729) = 1;
    void *definition = invoke<void *>(shape, 4);
    invoke(shape, 5, rotation);
    auto bind_pose = [&] {
        std::memcpy(bytes(body) + 680, position, 12);
        std::memcpy(bytes(body) + 692, rotation, 36);
        matrices::transpose(reinterpret_cast<float *>(bytes(body) + 692),
                            reinterpret_cast<float *>(bytes(body) + 72));
    };
    bind_pose();
    const float mass = at<float>(definition, 40) > 20.f ? at<float>(definition, 40) : 20.f;
    at<float>(body, 32) = mass;
    for (unsigned axis = 0; axis < 3; ++axis)
        at<Dword>(body, 120 + axis * 4) = at<Dword>(definition, 56 + axis * 4);
    for (unsigned axis = 0; axis < 3; ++axis) {
        float &extent = at<float>(body, 120 + axis * 4);
        extent = extent > 0.15f ? extent : 0.15f;
    }
    initialize_body_mass(body, mass, at<float>(body, 120), at<float>(body, 124),
                         at<float>(body, 128), at<int>(definition, 48),
                         at<Dword>(definition, 52));
    bind_pose();
    for (unsigned offset = 8; offset <= 28; offset += 4)
        at<Dword>(body, offset) = 0;
    reset_body_ground(body, 16);
    bind_pose();
    clear_ground_contact(bytes(body) + 508);
    at<void *>(body, 0) = shape;
    at<Byte>(body, 5) = 1;
    at<Dword>(body, 676) = 0;
    at<float>(body, 60) = at<float>(definition, 44) > 1.f ? at<float>(definition, 44) : 1.f;
    float &friction = at<float>(body, 56);
    if (friction < 0)
        friction = 0;
    else if (friction > 1.f)
        friction = 1.f;
    at<Dword>(body, 48) = 0;
    return 1;
}

void reset_body_ground(void *body, Byte flags) {
    clear_ground_contact(bytes(body) + 508);
    std::memcpy(bytes(body) + 492, bytes(body) + 680, 12);
    if (at<Byte>(body, 728)) {
        const auto z = static_cast<long double>(at<float>(body, 16));
        at<Dword>(body, 504) = 0;
        const auto x = static_cast<long double>(at<float>(body, 8));
        at<Dword>(body, 488) = 0;
        const float square = float(x * x + z * z);
        at<float>(body, 592) = float(__builtin_sqrtl(static_cast<long double>(square)));
        pending::update_ground_state(body);
        project_velocity(body);
    }
    auto *rotation = reinterpret_cast<float *>(bytes(body) + 692);
    auto *inverse = reinterpret_cast<float *>(bytes(body) + 72);
    if (!(flags & 16)) {
        std::memcpy(rotation, saved::identity_matrix, 36);
        matrices::transpose(rotation, inverse);
    }
    matrices::transpose(rotation, inverse);
    at<Dword>(body, 108) = 0;
    at<float>(body, 112) = 1.f;
    at<Dword>(body, 116) = 0;
    locate_ground(bytes(body) + 508, reinterpret_cast<float *>(bytes(body) + 680),
                  reinterpret_cast<float *>(bytes(body) + 484));
    project_velocity(body);
}
} // namespace nfs::game::runtime::physics
namespace nfs::game::runtime::ai::lifecycle::pending {

Dword bind_shape_body(void *shape, Dword flags) {
    void *body = physics::reserve_vehicle();
    at<void *>(shape, 80) = body;
    if (body) {
        if (physics::initialize_shape_body(body, shape, Byte(flags))) {
            if (at<Byte>(shape, 116))
                at<Dword>(body, 60) = 0;
            const char *name = reinterpret_cast<const char *>(invoke<void *>(shape, 4)) + 80;
            if (!_stricmp(name, "sign"))
                at<float>(body, 48) = at<float>(body, 124) * 0.5f;
            return 1;
        }
        physics::release_vehicle(
            body); // Caller repeats release after the failing initializer.
    }
    return 0;
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
