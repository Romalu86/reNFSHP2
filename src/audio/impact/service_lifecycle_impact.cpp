#include "../race/race_audio_distance.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "nfs/network/session/network_rules.hpp"
#include <cstring>

namespace nfs::services::pending {
alignas(4) Byte boom_sound_definitions[9628] = {
#include "../world/track_object_sound_prefix.inc"
#include "service_lifecycle_impact_data.inc"
};
Byte (&impact_definition_data)[9364] =
    *reinterpret_cast<Byte (*)[9364]>(boom_sound_definitions + 264);
} // namespace nfs::services::pending

namespace nfs::services {
namespace {
int surface_event(int surface) {
    switch (surface) {
    case 0: {
        constexpr int choices[] = {82, 28, 84, 85};
        return choices[network::rules::random() & 3];
    }
    case 2:
        return 59;
    case 3:
        return (network::rules::random() & 1) + 37;
    case 6:
        return (network::rules::random() & 1) + 80;
    case 7:
        return 100;
    case 8:
    case 9:
        return (network::rules::random() & 1) + 104;
    case 10:
        return (network::rules::random() & 1) + 98;
    case 11:
        return (network::rules::random() & 1) + 96;
    case 12:
        return 106;
    case 13:
        return 101;
    default:
        return 86;
    }
}
int selector_event(unsigned selector, float strength, float collision_measure) {
    unsigned row;
    const auto *table = pending::impact_definition_data + 9100 + selector * 44;
    if (selector != 6) {
        row = strength > 8.5f   ? 0
              : strength > 6.5f ? 1
              : strength > 4.5f ? 2
              : strength > 2.5f ? 3
                                : 4;
        if (field<Dword>(table, 40))
            row = network::rules::random() % 5;
    } else {
        table = pending::impact_definition_data + 9100;
        row = collision_measure > 1000.0f  ? 0
              : collision_measure > 500.0f ? 1
              : collision_measure > 100.0f ? 2
              : collision_measure > 30.0f  ? 3
                                           : 4;
    }
    return field<int>(table, (row * 2 + (network::rules::random() & 1)) * 4);
}
} // namespace


void submit_impact(const float *position, int first_material, int second_material, float strength,
                   float collision_measure) {
    namespace runtime = game::runtime;
    namespace audio = game::audio;
    namespace playback = game::runtime::race_audio;
    using playback::pending::race_audio_listener_count;
    using playback::pending::listener_index;
    Dword one_shot = 0;
    if (audio::state::world<Byte>(3108 + listener_index * 2760)) {
        auto *camera = runtime::invoke<Dword *>(game::tick_services[listener_index], 2);
        const Dword mode = field<Dword>(camera, 72);
        if (mode == 9 || mode == 6)
            return;
    }
    for (int listener = 0; listener < race_audio_listener_count; ++listener) {
        auto *camera = runtime::invoke<Dword *>(game::tick_services[listener], 2);
        struct ListenerTransform {
            Dword position[3];
            float orientation[23];
        } transform;
        playback::camera_position(camera, transform.position);
        playback::camera_orientation(camera, transform.orientation);
        if (strength < 1.0f)
            return;
        const float clamped_strength = strength > 10.0f ? 10.0f : strength;
        int distance_units = playback::pending::rounded_point_distance(
            reinterpret_cast<const float *>(transform.position), position);
        if (distance_units < 0 || distance_units > 10000)
            distance_units = 10000;
        const int squared_distance = distance_units * distance_units;
        int pan = static_cast<int>(playback::pending::spatial_pan(&transform, position));
        if (race_audio_listener_count == 2)
            pan = listener ? 16383 : 49152;
        const Dword channel = listener ? 256 : 0;
        int volume = 0;
        if (squared_distance < 900) {
            const void *rules = lifecycle::manager->slots[72 / 4];
            const int numerator_range = field<volatile Dword>(rules, 140) ? 10000 : 5625;
            const int denominator = field<volatile Dword>(rules, 140) ? 10000 : 5625;
            volume = (numerator_range - squared_distance) * 127 / denominator;
        }
        const bool outside_rear = pan <= 16384 || pan >= 49152;
        const bool near_collision = outside_rear || squared_distance < 100;
        int material = first_material;
        if (first_material >= 15 && first_material < 200) {
            if (second_material >= 15 && second_material < 200) {
                if (first_material > 18)
                    continue;
                unsigned selector = 6;
                if (first_material == 15) {
                    switch (second_material) {
                    case 20:
                        selector = 2;
                        break;
                    case 21:
                        selector = 4;
                        break;
                    case 19:
                        selector = 1;
                        break;
                    case 8:
                        selector = 5;
                        break;
                    case 15:
                        selector = 0;
                        break;
                    }
                } else if (first_material == 18 && second_material == 19) {
                    selector = 3;
                }
                const int event = selector_event(selector, clamped_strength, collision_measure);
                if (first_material != 18 && second_material != 19) {
                    int extra_event = -1;
                    if (clamped_strength > 8.5f) {
                        if (network::rules::random() % 100 > 85)
                            extra_event = (network::rules::random() & 3) + 55;
                    } else if (network::rules::random() % 100 > 96) {
                        extra_event = 56;
                    }
                    if (extra_event != -1 && near_collision)
                        playback::pending::play_event(channel | 116, extra_event, 64,
                                                               volume / 2, pan, one_shot != 0);
                }
                if (event != -1 && near_collision)
                    playback::pending::play_event(channel | 116, event, 64, volume, pan,
                                                           one_shot != 0);
                continue;
            }
            material = second_material;
        }
        int event;
        if (material >= 200) {
            const int definition = audio::state::track<int>(11400u + Dword(material) * 4u);
            if (definition == -1) {
                event = surface_event(material);
            } else {
                if (definition < 0 || definition >= 30)
                    continue;
                const Byte *record = pending::impact_definition_data + definition * 312;
                event = field<int>(record, 0);
                const Dword variants = field<Dword>(record, 4);
                if (event == -1 || !variants)
                    continue;
                event = static_cast<int>(static_cast<Dword>(event) +
                                         network::rules::random() % variants);
                const int radius = field<int>(record, 8), gain = field<int>(record, 12);
                if (gain == 128) {
                    one_shot = 1;
                    volume = 127;
                } else {
                    const int squared_radius =
                        static_cast<int>(static_cast<Dword>(radius) * static_cast<Dword>(radius));
                    volume = squared_distance < squared_radius && squared_radius != 0
                                 ? static_cast<int>(
                                       (static_cast<Dword>(squared_radius) - squared_distance) *
                                       static_cast<Dword>(gain)) /
                                       squared_radius
                                 : 0;
                }
            }
        } else {
            event = surface_event(material);
        }
        if (event != -1 && outside_rear)
            playback::pending::play_event(channel | 118, event, 64, volume, pan,
                                                   one_shot != 0);
    }
}
} // namespace nfs::services

namespace nfs::game::runtime::physics::pending {
void submit_impact_entry(const float *position, Dword first_material, Dword second_material,
                            float strength, Dword collision_bits) {
    float collision_measure;
    std::memcpy(&collision_measure, &collision_bits, sizeof(collision_measure));
    services::submit_impact(position, static_cast<int>(first_material),
                            static_cast<int>(second_material), strength, collision_measure);
}
} // namespace nfs::game::runtime::physics::pending
