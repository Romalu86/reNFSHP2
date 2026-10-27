#include "player_drive_runtime.hpp"
#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "../../math/vectors/retained_vector.hpp"
namespace nfs::game::runtime::ai::cars {

void drive_player_cinematic_path(void *entity, Dword controls) {
    void *car = at<void *>(entity, 368);
    Dword node = 0;
    if (!car || !at<void *>(car, 1768))
        return;
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    float lookahead = at<float>(car, 7948);
    if (lookahead < 0)
        lookahead = -lookahead;
    float forward[3], direction[3];
    std::memcpy(forward, bytes(car) + 3500, 12);
    void *section = invoke<void *>(entity, 1);
    int sample = invoke<int>(entity, 2);
    float alignment = 1.f;
    if (section && sample >= 0) {
        track::path_direction(section, sample, direction);
        alignment = float(E(forward[0]) * direction[0] + E(forward[1]) * direction[1] -
                          E(forward[2]) * direction[2]);
        if (at<int>(entity, 240) == -1)
            alignment = -alignment;
    }
    section = invoke<void *>(entity, 1);
    sample = invoke<int>(entity, 2);
    if (section && sample >= 0)
        track::path_direction(section, sample, forward);
    const bool reverse = alignment < 0;
    (void)track::retained_vehicle_direction(entity);
    if (reverse)
        lookahead = -lookahead;
    at<Byte>(at<void *>(car, 1768), 728) = 1;
    auto *requested = reinterpret_cast<float *>(bytes(entity) + 640);
    *requested = float_word(1132862440);
    float target[3], pose[21];
    const float *position = invoke<const float *>(entity, 0);
    if (!entities::pending::sample_path_target(bytes(entity) + 220, target, requested, &node, pose,
                                               lookahead, position, 0)) {
        invoke(entity, 33, 0.f, controls);
        if (invoke<long double>(entity, 22) < float_word(1055187230) && lookahead < 0 &&
            at<void *>(game::tick_services[0], 300) == car) {
            for (int wheel = 0; wheel < 4; ++wheel)
                particles::lifecycle::release_wheel_skid(car, wheel);
            float step[3], new_position[3];
            std::memcpy(step, bytes(car) + 3500, 12);
            void *body = at<void *>(car, 1768);
            for (unsigned axis = 0; axis < 3; ++axis)
                new_position[axis] = float(E(step[axis]) + at<float>(body, 680 + axis * 4));
            void *boundary = track::boundary(system(36), 1, 0);
            if (!track::contains(static_cast<Dword *>(boundary), new_position)) {
                for (unsigned axis = 0; axis < 3; ++axis) {
                    auto value = E(step[axis]);
                    if (value < E(-1))
                        value = E(-1);
                    else if (E(1) < value)
                        value = E(1);
                    step[axis] = float(value);
                    new_position[axis] = float(E(step[axis]) + at<float>(body, 680 + axis * 4));
                }
            }
            at<Byte>(at<void *>(car, 1768), 728) = 0;
            std::memcpy(bytes(body) + 680, new_position, 12);
            at<std::uint16_t>(car, 7940) = 1;
        }
        return;
    }
    *requested = float_word(1132862440);
    invoke(entity, 54, target, target, float_word(1132862440), requested, 2.f);
    *requested = float_word(1132862440);
    limit_player_turn_speed(entity, controls, target, float_word(1132862440));
}
} // namespace nfs::game::runtime::ai::cars
