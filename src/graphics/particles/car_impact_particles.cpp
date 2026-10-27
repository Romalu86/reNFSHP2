#include "../../math/vectors/retained_vector.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::game::world::camera {
void active_camera_position(void *, float *);
}
namespace nfs::game::runtime::physics::contacts_pending {
// Production callers discard incidental EAX.
void emit_sparks(void *car, const float *position, float strength, const float *normal) {
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    if (!audio::vehicle::car_audio_suppressed(car)) {
        float closest = -1;
        void *body = at<void *>(car, 1768);
        const int cameras = split_screen() ? 2 : 1;
        for (int camera = 0; camera < cameras; ++camera) {
            float eye[3];
            world::camera::active_camera_position(tick_services[camera], eye);
            const auto distance = retained_point_distance(eye, &at<float>(body, 680));
            if (distance < kernel::numeric::Extended(closest, kernel::numeric::current_format()) ||
                closest < 0)
                closest = float(distance);
        }
        if (closest > 150)
            return;
    }
    void *definition = particles::lifecycle::particle_prefix;
    while (at<Dword>(definition, 4) && at<Dword>(definition, 4) != 2)
        definition = bytes(definition) + 152;
    if (!at<Dword>(definition, 4))
        return;
    void *body = at<void *>(car, 1768);
    auto amount = E(strength);
    if (amount < E(0))
        amount = E(0);
    else if (E(1) < amount)
        amount = E(1);
    int count = int(Dword((Extended::integer(at<int>(definition, 8), format) * amount * amount)
                             .truncate_to_integer()));
    if (count < 2)
        count = 1;
    float velocity[3] = {at<float>(body, 8), at<float>(body, 12), -at<float>(body, 16)};
    float origin[3] = {position[0], position[1], position[2]};
    float direction[3];
    if (normal) {
        direction[0] = normal[0];
        direction[1] = normal[1];
        direction[2] = -normal[2];
    }
    // Both branches overwrite all three entries before normalization.
    const unsigned direction_offset = at<Dword>(body, 636) == 2 ? 624 : 8;
    for (unsigned axis = 0; axis < 3; ++axis)
        direction[axis] = at<float>(body, direction_offset + axis * 4);
    pending::normalize_collision_entry(direction);
    direction[2] = -direction[2];
    float impact[3] = {at<float>(body, 612), at<float>(body, 616), at<float>(body, 620)};
    pending::normalize_collision_entry(impact);
    impact[2] = -impact[2];
    for (int index = 0; index < count; ++index) {
        float scattered[3];
        for (unsigned axis = 0; axis < 3; ++axis) {
            const Dword random = emitter_random_word() & 65535;
            scattered[axis] =
                float(Extended::integer(random, format) * 0.0000152587890625f * 6.f + velocity[axis] - 3.f);
        }
        const Dword random = emitter_random_word() & 65535;
        const float size = float(Extended::integer(random, format) * 0.0000152587890625f * .25f + .75f);
        void *particle = particles::lifecycle::emit_car_particle(
            definition, origin, size, direction, impact, scattered, 1, 0);
        if (particle) {
            const std::int16_t life = at<std::int16_t>(particle, 4);
            const Dword variation = emitter_random_word();
            at<std::uint16_t>(particle, 4) =
                std::uint16_t(Dword(life) - variation % Dword(int(life) >> 1));
            at<Byte>(particle, 85) = Byte(emitter_random_word() % 3);
        }
    }
}

void notify_ground_impact(void *car, float speed) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return kernel::numeric::Extended(value, format); };
    const Byte material = at<Dword>(car, 2908) ? at<Byte>(car, 2962) & 31 : 0;
    const Dword kind = at<Dword>(pending::surface_properties(material), 0);
    switch (kind) {
    case 0:
    case 5:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 13:
    case 14:
    case 15:
    case 19:
    case 20:
    case 21:
    case 22:
    case 29:
    case 30:
        if (speed > .44703f) {
            void *body = at<void *>(car, 1768);
            float position[3] = {at<float>(body, 680),
                                 float(E(at<float>(body, 684)) - at<float>(body, 124)),
                                 -at<float>(body, 688)};
            const float normal[3] = {0, 1, 0};
            float strength = float(E(speed) * .2f);
            if (strength < 1)
                strength = 1;
            emit_sparks(car, position, strength, normal);
        }
    }
    const Dword base = speed <= 5 ? (speed <= 3 ? 40000 : 50000) : 65535;
    auto rule = [&](Dword key) { return network::packets::rule_value(system(72), key); };
    const int scaled_first = int(rule(53) * base) / 100;
    const float scaled_second = float(int(rule(53) * base) / 100);
    const int first = int(rule(55) * Dword(scaled_first)) / 100;
    const int second = int(rule(55) * Dword(int(scaled_second))) / 100;
    if (at<int>(car, 7996) <= first)
        at<int>(car, 7996) = first;
    if (at<int>(car, 8000) <= second)
        at<int>(car, 8000) = second;
    pending::submit_impact_entry(&at<float>(at<void *>(car, 1768), 680), 18, 19, speed,
                                    0);
}
} // namespace nfs::game::runtime::physics::contacts_pending
