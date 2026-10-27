#include "police_chase_x87.hpp"
#include "../../network/session/network_pursuit_runtime.hpp"
#include "../arrest/arrest_cinematic_runtime.hpp"
#include "../driving/cop_driving_runtime.hpp"
#include "../../ai/lifecycle/ai_chase_tuning.hpp"
#include "../../ai/driving/ai_car_behavior.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "../../physics/bodies/shape_body_runtime.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../ai/driving/ai_path_frame_x87.hpp"
#include "../../race/rules/race_rules_x87.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::world::camera { void state_orientation(void *, float *); }
namespace nfs::game::runtime::ai::cars {
void release_cop_reservation(void *, void *);
void update_cop_patrol_target(void *, Dword);
void *contact_entity(void *, int);
}
namespace nfs::game::runtime::ai::lifecycle::pending {
extern float chase_transition_seconds;
void *construct_path_reference(void *, void *, Dword);
void select_path_section(void *, void *);
}
namespace nfs::game::runtime::ai::entities::pending {
void steer_helicopter_patrol(void *);
void steer_helicopter_pursuit(void *);
void fire_helicopter_projectiles(void *);
}

namespace nfs::game::runtime::ai::lifecycle::pending::police_frame {
void marker_rotation(void *location, float *position, float *rotation) {
    const float *marker = at<const float *>(location, 12);
    std::memcpy(position, marker, 12);
    world::quaternion_matrix(marker + 3, rotation);
    for (unsigned component : {2u, 5u, 6u, 7u})
        rotation[component] *= -1.f;
}
void displaced_position(void *entity, const float *offset, float *position) {
    const float *source = invoke<const float *>(entity, 0);
    position[0] = float(static_cast<long double>(source[0]) + offset[0]);
    position[1] = float(static_cast<long double>(source[1]) + offset[1]);
    position[2] = float(static_cast<long double>(offset[2]) - source[2]);
}
}

namespace nfs::game::runtime::ai::lifecycle::pending::police_frame {
Dword car_speed_phrase(void *car) {
    return Dword(
        police_math::truncate((static_cast<long double>(at<float>(car, 3168)) * saved_float(1074735832))));
}
Dword car_sound_location(void *car) {
    float position[3];
    invoke(car, 0, position);
    position[2] = -position[2];
    return sound_location(position);
}
void announce(void *owner, Dword event, int listener, Dword speed, int units, int location) {
    speech::game::dispatch_police_event(owner, event, listener, 0, 0, 0, 0, 0, speed, units, 0,
                                        location);
}
void reset_ground_announcements(void *owner) {
    at<Dword>(owner, 44) = 0;
    at<Dword>(owner, 48) = 100000;
}
}

namespace nfs::game::runtime::ai::lifecycle::pending::police_frame {
void *allocate_police_chase_retained(void *owner, void *cop, void *target, Byte forced_announcement) {

    void *racer = invoke<void *>(entities::interface_at(target, 2), 2);
    if (!racer || at<Byte>(racer, 596))
        return nullptr;
    const Dword player = invoke<Dword>(target, 9);
    void *autonomous = application::pending::traffic_actor_view(cop);
    if ((state<Byte>(7352731) || state<Byte>(7352730)) && player)
        return nullptr;
    if (at<std::uint16_t>(owner, 88) > 1)
        return nullptr;
    void *chase = nullptr;
    for (unsigned address = 7352496; address < 7352728; address += 116) {
        void *candidate = &state<Dword>(address);
        if (!chase_active(candidate)) {
            chase = candidate;
            break;
        }
    }
    const Dword serial = state<Dword>(7352740);
    state<Dword>(7352740) = serial + 1;
    at<Dword>(chase, 64) = serial;
    ++at<std::uint16_t>(owner, 88);
    if (player)
        state<Byte>(7352730) = 1;
    Dword level = 0;
    const Dword participant = invoke<Dword>(target, 11);
    if (!participant)
        level = 1;
    else {
        const int difficulty = at<int>(system(72), 192);
        Dword threat = at<Dword>(entities::interface_at(target, 4), 12);
        if (roadblock_start_level != Dword(-1))
            threat = roadblock_start_level - 1;
        if (difficulty == 0)
            level = threat < 2 ? 1 : 3 - Dword(threat < 4);
        else if (difficulty == 1)
            level = threat < 2 ? 2 : 4 - Dword(threat < 4);
        else if (difficulty == 2)
            level = threat == 0 ? 3 : 5 - Dword(threat < 3);
    }
    begin_police_chase(chase, cop, target, level);
    if (!autonomous)
        return chase;
    if (participant && roadblock_start_level != Dword(-1) && roadblock_start_strategy != Dword(-1))
        at<std::uint16_t>(chase, 8) = std::uint16_t(roadblock_start_strategy);
    switch (level) {
    case 1: {
        const auto strategy = std::uint16_t(ai::random_word() % 3);
        at<std::uint16_t>(chase, 8) = strategy;
        if (strategy == 0 || strategy == 1)
            at<Dword>(autonomous, 580) = 7;
        else if (strategy == 2)
            at<Dword>(autonomous, 580) = 1;
        break;
    }
    case 2: {
        const auto strategy = std::uint16_t(ai::random_word() % 3);
        at<std::uint16_t>(chase, 8) = strategy;
        if (strategy == 0 || strategy == 1)
            at<Dword>(autonomous, 580) = 7;
        else if (strategy == 2)
            at<Dword>(autonomous, 580) = 1;
        at<std::uint16_t>(chase, 12) = strategy == 1 ? 1 : 0;
        at<Dword>(chase, 16) = Dword(-1);
        at<Byte>(chase, 20) = 0;
        break;
    }
    case 3:
        choose_chase_level_three(owner, chase, autonomous);
        break;
    case 4:
        choose_chase_level_four(owner, chase, autonomous);
        break;
    case 5:
        choose_chase_level_five(owner, chase, autonomous);
        break;
    }
    if (!at<std::uint16_t>(chase, 12) && at<int>(chase, 16) == -1)
        at<Byte>(chase, 108) = 1;
    position_chase_cops(chase, 1, at<int>(chase, 40));
    void *perpetrator = at<void *>(chase, 44);
    const Dword threat = at<Dword>(entities::interface_at(perpetrator, 4), 12);
    float position[3];
    invoke(at<void *>(cop, 368), 0, position);
    position[2] = -position[2];
    sound_location(position);
    at<Dword>(system(84), 84) = threat;
    perpetrator = at<void *>(chase, 44);
    const Dword speed =
        Dword(police_math::truncate(static_cast<long double>(at<float>(at<void *>(perpetrator, 368), 3168)) *
              cars::float_word(1074735832)));
    int voice = -1;
    if (at<int>(system(72), 176) == 1) {
        if (invoke<Dword>(perpetrator, 11)) {
            select_police_voice(system(84), 0, at<Dword>(cop, 564));
            voice = at<int>(system(84), 64);
            at<Byte>(system(84), 89) = 1;
        } else {
            select_police_voice(system(84), 1, at<Dword>(cop, 564));
            voice = at<int>(system(84), 68);
            if (at<Byte>(system(84), 89))
                voice = -1;
        }
    } else if (invoke<Dword>(perpetrator, 11)) {
        if (at<void *>(chase, 44) == at<void *>(system(76), 84164)) {
            select_police_voice(system(84), 0, at<Dword>(cop, 564));
            voice = at<int>(system(84), 64);
        } else {
            select_police_voice(system(84), 1, at<Dword>(cop, 564));
            voice = at<int>(system(84), 68);
        }
    }
    const Dword event = forced_announcement ? 3 : 1 + Dword(threat > 1);
    if (voice != -1) {
        void *announced = at<void *>(chase, 44);
        const Dword colour = police_colour_mask(at<int>(announced, 364));
        void *speech_owner = system(84);
        const Dword model = car_speech_mask(at<int>(announced, 356));
        speech::game::dispatch_police_event(speech_owner, event, voice, 0, 0, 0, int(colour),
                                            int(model), speed, 2, 0, 0);
    }
    return chase;
}
}

namespace nfs::game::runtime::ai::lifecycle::pending::police_frame {
void begin_police_chase_retained(void *chase, void *cop, void *target, Dword level) {

    const Dword duration = level >= 1 && level <= 5 ? pursuit_level_seconds[level - 1] : 0;
    const Dword ticks = duration * 60;
    at<Dword>(chase, 68) = ticks;
    at<Dword>(chase, 84) =
        Dword(police_math::truncate(static_cast<long double>(ticks) * chase_transition_seconds));
    reset_police_chase(chase);
    Dword delay = state<Dword>(7352472);
    if (state<Dword>(7352468) != delay) {
        const Dword random = ai::random_word();
        const Dword minimum = state<Dword>(7352472);
        const Dword maximum = state<Dword>(7352468);
        delay += random % (maximum - minimum + 1);
    }
    const Dword tick = now();
    at<Dword>(chase, 24) = tick;
    at<Dword>(chase, 100) = delay;
    at<void *>(chase, 44) = target;
    at<Dword>(chase, 88) = 1;
    at<Dword>(chase, 104) = tick + delay;
    at<Dword>(chase, 76) = tick;
    at<Dword>(chase, 80) = tick;
    // Keep the three adjacent native WORD stores separate.
    at<volatile std::uint16_t>(chase, 8) = 0;
    at<volatile std::uint16_t>(chase, 10) = 0;
    at<volatile std::uint16_t>(chase, 12) = 0;
    at<Dword>(chase, 16) = Dword(-1);
    at<Byte>(chase, 20) = 0;
    if (level > 5)
        level = 5;
    at<Dword>(chase, 52) = std::min(level, at<Dword>(chase, 56));
    if (application::pending::traffic_actor_view(cop))
        ++at<std::uint16_t>(chase, 10);
    if (at<Dword>(chase, 40) < 3) {
        at<void *>(chase, 28 + at<Dword>(chase, 40) * 4) = cop;
        ++at<Dword>(chase, 40);
        invoke(cop, 6, Dword(1));
        invoke(perp_interface(cop), 1, chase);
    }
    invoke(perp_interface(target), 1, chase);
}
}

namespace nfs::game::runtime::ai::lifecycle::pending::police_frame {
void update_police_chase_retained(Dword *owner, void *chase) {

    void *initial_target = at<void *>(chase, 44);
    if (at<Byte>(initial_target, 596))
        entities::pending::set_police_chase_outcome(chase, 2);
    if (invoke<Dword>(at<void *>(chase, 44), 11))
        update_chase_announcements(system(84), chase);
    if (needs_chase_backup(chase)) {
        if (at<Dword>(chase, 16) != Dword(-1) && !request_roadblock(owner, chase)) {
            at<Dword>(owner, 148) = 0;
            at<Dword>(chase, 16) = Dword(-1);
        }
        for (int remaining = at<std::int16_t>(chase, 12); remaining > 0; --remaining) {
            const Byte assigned = request_backup_car(chase, nullptr);
            void *target = at<void *>(chase, 44);
            if (invoke<Dword>(target, 11) && remaining == at<std::int16_t>(chase, 12) && assigned)
                speech::game::dispatch_police_event(
                    system(84), 25, player_target_listener(target), 0, 0, 0, 0, 0, 0, 2, 0, -1);
        }
        at<Byte>(chase, 108) = 1;
    }
    if (at<Dword>(chase, 96) != Dword(-1))
        update_spike_announcement(owner, chase);
    if (at<Byte>(chase, 20))
        update_chase_helicopter(owner, chase);
    const Dword cop_count = at<Dword>(chase, 40);
    void *target = invoke<void *>(entities::interface_at(at<void *>(chase, 44), 2), 1);
    auto retained_speed =
        static_cast<long double>(invoke<long double>(target, 22)) *
        saved_float(1074735832);
    if (retained_speed < static_cast<long double>(0.f))
        retained_speed = -retained_speed;
    const float speed = float(retained_speed);
    const float bust_distance = at<float>(system(76), 84552);
    const Byte slow = update_bust_speed(perp_interface(initial_target), speed);
    Dword threats = 0, near_cops = 0;
    for (Dword index = 0; index < (cop_count & 65535); ++index) {
        void *cop = index < at<Dword>(chase, 40) ? at<void *>(chase, 28 + index * 4) : nullptr;
        if (slow) {
            if (ai::pair_distance(initial_target, cop) < bust_distance)
                ++near_cops;
            ++threats;
        } else if (cop_threat_zone(at<void *>(chase, 44), cop) != 4)
            ++threats;
    }
    update_bust_timer(perp_interface(initial_target), speed, int(near_cops) > 0);
    if (near_cops && bust_timer_expired(perp_interface(initial_target))) {
        entities::pending::set_police_chase_outcome(chase, 1);
        return;
    }
    if (update_chase_meter(chase, std::int16_t(threats)) == 2) {
        void *cop = at<Dword>(chase, 40) ? at<void *>(chase, 28) : nullptr;
        void *perp = at<void *>(chase, 44);
        const Dword target_slot = perp ? network::world::vehicle_slot(perp) : Dword(-1);
        const Dword cop_slot = cop ? network::world::vehicle_slot(cop) : Dword(-1);
        invoke(at<void *>(system(92), 4), 32, cop_slot, target_slot, at<Dword>(chase, 52));
        finish_chase(owner, chase);
    }
}
}

namespace nfs::game::runtime::ai::lifecycle::pending::police_frame {
Byte needs_chase_backup_retained(void *chase) {

    if (at<Byte>(chase, 108))
        return 0;
    const auto ratio = static_cast<long double>(at<int>(chase, 72)) /
                       static_cast<long double>(at<int>(chase, 68));
    if (ratio < static_cast<long double>(.5f))
        return 1;
    return at<int>(chase, 104) <= int(now());
}
}

namespace nfs::game::runtime::ai::lifecycle::pending::police_frame {
Byte bust_timer_expired_retained(void *perp) {

    const int start = at<int>(perp, 16);
    if (start <= 0)
        return 0;
    const Dword delay =
        Dword(police_math::truncate(static_cast<long double>(at<float>(system(76), 84560)) * 60.f));
    return start < int(now() - delay);
}
}

namespace nfs::game::runtime::ai::lifecycle::pending::police_frame {
Dword cop_threat_zone_retained(void *target, void *cop) {

    const Dword duration = invoke<Dword>(at<void *>(system(92), 4), 23);
    const float range = at<float>(system(76), 84544);
    const float factor =
        duration ? float(static_cast<long double>(1.f) - static_cast<long double>(at<int>(target, 576)) /
                                                     static_cast<long double>(int(duration)) *
                                                     at<float>(system(76), 84548))
                 : 1.f;
    const auto distance = static_cast<long double>(ai::pair_distance(target, cop));
    const auto limit = static_cast<long double>(factor) * range;
    return limit >= distance ? 1 : 4;
}
}

namespace nfs::game::runtime::ai::lifecycle::pending::police_frame {
Dword place_roadblock_cop_retained(void *location, void *cop) {

    const Dword previous_count = at<Dword>(location, 32);
    float axis[3]{}, position[3], rotation[9];
    // Initialize the stack word with the incoming pointer bits.
    float separation = saved_float(Dword(reinterpret_cast<std::uintptr_t>(cop)));
    if (previous_count == 1) {
        void *first_cop = at<void *>(location, 36);
        std::memcpy(axis, bytes(entity_body(first_cop)) + 716, 12);
        const float half_length = at<float>(entity_body(cop), 128);
        const auto shift = (static_cast<long double>(half_length) + half_length + roadblock_spacing) * .5f;
        float offset[3];
        for (unsigned component = 0; component != 3; ++component)
            offset[component] = float(static_cast<long double>(axis[component]) * shift);
        displaced_position(first_cop, offset, position);
        position_entity(first_cop, position,
                        reinterpret_cast<float *>(bytes(entity_body(first_cop)) + 692), 0);
        const float first_length = at<float>(entity_body(first_cop), 128);
        separation = float(static_cast<long double>(first_length) + first_length + roadblock_spacing);
    }
    if (int(previous_count) < 2) {
        at<void *>(location, 36 + previous_count * 4) = cop;
        ++at<Dword>(location, 32);
    }
    marker_rotation(location, position, rotation);
    if (at<Dword>(location, 32) == 2) {
        for (unsigned component : {0u, 1u, 2u, 6u, 7u, 8u})
            rotation[component] = -rotation[component];
        const auto shift = static_cast<long double>(separation) * -.5f;
        position[0] = float(static_cast<long double>(position[0]) + static_cast<long double>(axis[0]) * shift);
        for (unsigned component = 1; component != 3; ++component)
            position[component] = float(static_cast<long double>(position[component]) +
                                        float(static_cast<long double>(axis[component]) * shift));
    }
    position_entity(cop, position, rotation, 0);
    if (void *car = at<void *>(cop, 368)) {
        const Dword slot = at<Dword>(car, 1772);
        void *record = slot < 24 ? bytes(system(72)) + 248 + slot * 84 : nullptr;
        const Dword grid_index = at<Dword>(record, 20);
        if (grid_index < 8) {
            std::memcpy(bytes(system(68)) + 700 + grid_index * 12, position, 12);
            std::memcpy(bytes(system(68)) + 988 + grid_index * 36, rotation, 36);
        }
    }
    invoke(cop, 6, 1);
    ai::pending::dispatch_vehicle_event(static_cast<Dword *>(cop), 9);
    return at<Dword>(location, 4);
}
}

namespace nfs::game::runtime::ai::lifecycle::pending::police_frame {
void place_roadblock_spikes_retained(void *location, void *spike) {

    float axis[3]{}, offset[3]{}, separation = 0.f;
    if (at<Dword>(location, 32)) {
        std::memcpy(axis, bytes(entity_body(at<void *>(location, 36))) + 716, 12);
        const float width =
            float(static_cast<long double>(spike_strip_size) * at<float>(entity_body(spike), 120));
        const auto shift = (static_cast<long double>(width) + width + roadblock_spacing) * .5f;
        for (unsigned component = 0; component != 3; ++component)
            offset[component] = float(static_cast<long double>(axis[component]) * shift);
    }
    for (Dword index = 0, remaining = at<Dword>(location, 32); remaining; ++index, --remaining) {
        void *cop = at<void *>(location, 36 + index * 4);
        float position[3];
        displaced_position(cop, offset, position);
        position_entity(cop, position, reinterpret_cast<float *>(bytes(entity_body(cop)) + 692),
                        0);
        const float length = at<float>(entity_body(cop), 128);
        separation = float(static_cast<long double>(length) + length + roadblock_spacing + separation);
    }
    at<Byte>(entity_body(spike), 729) = 1;
    float position[3], rotation[9];
    marker_rotation(location, position, rotation);
    at<void *>(location, 44) = spike;
    at<void *>(spike, 284) = location;
    float reordered[9]{rotation[6], rotation[7],  rotation[8],  rotation[3], rotation[4],
                       rotation[5], -rotation[0], -rotation[1], -rotation[2]};
    if (separation != 0.f) {
        const auto shift = static_cast<long double>(separation) * -.5f;
        position[0] = float(static_cast<long double>(position[0]) + static_cast<long double>(axis[0]) * shift);
        for (unsigned component = 1; component != 3; ++component)
            position[component] = float(static_cast<long double>(position[component]) +
                                        float(static_cast<long double>(axis[component]) * shift));
    }
    position_roadblock_spikes(spike, position, reordered);
}
}

namespace nfs::game::runtime::ai::lifecycle::pending::police_frame {
Dword sound_location_retained(const float *position) {

    const int count = at<int>(system(24), 108);
    for (int index = 0; index < count; ++index) {
        Dword *marker = sound_location_marker(system(24), index);
        if (police_math::point_distance(position, reinterpret_cast<float *>(marker)) <
            static_cast<long double>(at<float>(marker, 12)))
            return at<Dword>(marker, 16);
    }
    return Dword(-1);
}
}

namespace nfs::game::runtime::ai::lifecycle::pending::police_frame {
void update_chase_announcements_retained(void *owner, void *chase) {

    const Dword elapsed = chase_elapsed_ticks(chase);
    const int cop_count = at<int>(chase, 40);
    void *cop = cop_count ? at<void *>(chase, 28) : nullptr;
    const int listener = player_chase_listener(chase);
    const int strategy = at<int>(chase, 16);
    if (elapsed > (at<Dword>(chase, 100) >> 1) && !at<Byte>(owner, 52)) {
        if (strategy != -1) {
            Dword event = 21;
            if (strategy == 0 || strategy == 2)
                event += runtime::emitter_random_word() % 100 > 40 ? 7 : 0;
            announce(system(84), event, listener, 0, 2, 0);
            at<Byte>(owner, 52) = 1;
        } else if (at<std::int16_t>(chase, 12)) {
            const Dword location = car_sound_location(at<void *>(cop, 368));
            announce(system(84), 0, listener, 0, 0, int(location));
            at<Byte>(owner, 52) = 1;
        }
    }
    const auto ratio = static_cast<long double>(at<int>(chase, 72)) /
                       static_cast<long double>(at<int>(chase, 68));
    if (ratio < static_cast<long double>(.15))
        return;
    void *target = invoke<void *>(entities::interface_at(at<void *>(chase, 44), 2), 1);
    void *car = at<void *>(target, 368);
    const Dword speed = car_speed_phrase(car);
    // TEST AH,41 / JE rejects ordered greater, while unordered values continue.
    if (ai::pair_distance(cop, target) > 75.f)
        return;
    if (int(elapsed) < 11) {
        reset_ground_announcements(owner);
        at<Dword>(owner, 36) = 0;
        at<Dword>(owner, 40) = 100000;
        at<Byte>(owner, 53) = 0;
    }
    if (at<Byte>(owner, 88) && !at<Byte>(owner, 90) && int(car_speed_phrase(car)) >= 16) {
        announce(owner, 20, listener, 0, 2, 0);
        at<Byte>(owner, 90) = 1;
        reset_ground_announcements(owner);
        return;
    }
    if (at<int>(owner, 64) != listener)
        return;
    if (at<float>(at<void *>(car, 1768), 112) < .5f && !at<Byte>(owner, 53)) {
        announce(owner, 11, listener, 0, 2, 0);
        at<Byte>(owner, 53) = 1;
        reset_ground_announcements(owner);
    }
    if (int(speed) < 60)
        return;
    if (int(elapsed) > 10 && !audio::state::option<Dword>(audio_layout::speech_stream_active) && !at<Dword>(owner, 44)) {
        at<Dword>(owner, 44) = elapsed;
        at<Dword>(owner, 48) = runtime::emitter_random_word() % 900 + 900;
        return;
    }
    if (int(elapsed - at<Dword>(owner, 44)) <= at<int>(owner, 48))
        return;
    const Dword location = car_sound_location(car);
    Dword probability = 0;
    if (at<int>(system(72), 176) == 1) {
        const int intensity = at<int>(owner, 84);
        if (intensity == 2)
            probability = 30;
        else if (intensity == 3)
            probability = 60;
        else if (intensity > 3)
            probability = 80;
    }
    Dword event;
    if (cop_count < 2 || runtime::emitter_random_word() % 100 < 51)
        event = runtime::emitter_random_word() % 100 > probability ? 10 : 23;
    else
        event = 24;
    announce(owner, event, listener, speed, 2, int(location));
    reset_ground_announcements(owner);
}
}

namespace nfs::game::runtime::ai::lifecycle::pending::police_frame {
void update_helicopter_announcement_retained(void *owner, void *chase) {

    const Dword elapsed = chase_elapsed_ticks(chase);
    const int listener = at<int>(system(84), 64);
    void *target = invoke<void *>(entities::interface_at(at<void *>(chase, 44), 2), 1);
    void *car = at<void *>(target, 368);
    if (!audio::state::option<Dword>(audio_layout::speech_stream_active) && !at<Dword>(owner, 36)) {
        at<Dword>(owner, 36) = elapsed;
        at<Dword>(owner, 40) = runtime::emitter_random_word() % 1200 + 300;
        return;
    }
    if (int(elapsed - at<Dword>(owner, 36)) > at<int>(owner, 40)) {
        const Dword speed = car_speed_phrase(car);
        const Dword location = car_sound_location(car);
        announce(owner, 16, listener, speed, 0, int(location));
        at<Dword>(owner, 36) = 0;
        at<Dword>(owner, 40) = 100000;
    }
}
}

namespace nfs::game::runtime::ai::lifecycle::pending::police_frame {
void activate_chase_helicopter_retained(void *helicopter, void *chase) {

    at<Byte>(helicopter, 349) = 1;
    at<Byte>(helicopter, 1660) = 0;
    at<Dword>(helicopter, 1668) = 0;
    at<float>(helicopter, 1672) = -1.f;
    at<Dword>(helicopter, 1676) = 0;
    at<Dword>(helicopter, 352) = 0;
    at<Dword>(helicopter, 344) = 0;
    at<Dword>(helicopter, 336) = ai::random_word() % 3;
    const Dword spin = ai::random_word();
    at<Dword>(helicopter, 340) = spin * 2 - 1;
    at<Dword>(helicopter, 1652) = now() + state<Dword>(7352836);
    at<void *>(helicopter, 408) = chase;
    at<Dword>(helicopter, 380) = 1;
    invoke(helicopter, 6, 1);
    at<const float *>(helicopter, 436) = invoke<const float *>(at<void *>(chase, 44), 0);
    entities::pending::select_helicopter_destination(helicopter);
    const Byte side = Byte(ai::random_word()) & 1;
    Dword route;
    int node;
    if (find_pursuit_spawn(at<void *>(chase, 44), side, 250.f, 250.f, &route, &node)) {
        float position[3];
        std::memcpy(position, section_position(reinterpret_cast<void *>(route), node), 12);
        position[1] = float(static_cast<long double>(5.5f) + position[1]);
        reset_helicopter_position(helicopter, position);
        int direction;
        float distance;
        Dword sample;
        if (entities::pending::target_route_branch(helicopter, &direction, &distance, &sample)) {
            void *cursor = bytes(helicopter) + 236;
            void *section = lifecycle::path_current_section(cursor);
            const Dword index = entities::path_sample_index(cursor);
            float tangent[3];
            dependencies::sample_horizontal_tangent(section, int(index), tangent);
            const auto angle = runtime::turn_direction_retained(tangent[0], -tangent[2]);
            at<float>(helicopter, 388) = float(angle);
            if (direction == -1)
                at<float>(helicopter, 388) = float(angle + .5f);
            const float heading = at<float>(helicopter, 388);
            if (heading >= .5f)
                at<float>(helicopter, 388) = float(static_cast<long double>(heading) - 1.f);
            else if (heading < -.5f)
                at<float>(helicopter, 388) = float(static_cast<long double>(heading) + 1.f);
            if (direction != at<int>(helicopter, 240))
                entities::pending::reverse_entity_path(helicopter);
        }
    }
    at<Dword>(helicopter, 364) = 1110626271;
    speech::game::dispatch_police_event(system(84), 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}
}

namespace nfs::game::runtime::ai::cars::police_frame {
using namespace lifecycle::pending;
Byte request_network_spikes_retained(void *entity) {

    void *chase = network_chase(entity);
    if (!chase)
        return 0;
    int listener = pursuit_listener(chase);
    if (listener == -1)
        listener = 0;
    if (!at<Dword>(entity, 568) && at<int>(chase, 96) == -1 &&
        idle_roadblock_cop_count(&lifecycle::state<Dword>(7353160)) > 1 &&
        free_spike_available(&lifecycle::state<Dword>(7353296))) {
        const Dword strategy = ai::random_word() % 3;
        Dword event = 21;
        at<Dword>(entity, 588) = strategy;
        if ((strategy == 0 || strategy == 2) && runtime::emitter_random_word() % 100 > 40)
            event = 28;
        if (!at<Byte>(system(84), 94 + listener))
            speech::game::dispatch_police_event(system(84), event, listener, 0, 0, 0, 0, 0, 0, 2,
                                                0, 0);
        if (record_support_request(at<void *>(system(92), 4),
                                   network::world::vehicle_slot(entity), 8)) {
            const auto delay = static_cast<long double>(at<float>(system(76), 84424)) *
                               -60.f;
            at<Dword>(entity, 568) = now() - Dword(police_math::truncate(delay));
            at<Byte>(entity, 574) = 1;
            at<Dword>(entity, 576) = now();
            return 1;
        }
        if (!at<Byte>(system(84), 94 + listener)) {
            speech::game::dispatch_police_event(system(84), 22, listener, 0, 0, 0, 0, 0, 0, 2, 0,
                                                0);
            at<Byte>(system(84), 94 + listener) = 1;
            return 0;
        }
    }
    play_unavailable_action(listener);
    return 0;
}
}

namespace nfs::game::runtime::ai::cars::police_frame {
using namespace lifecycle::pending;
Byte detect_speeding_racers_retained(void *entity) {

    using namespace lifecycle::pending;
    if (network_chase(entity) || at<Byte>(entity, 1804) || !population_loading)
        return 0;
    auto E = [](float value) { return static_cast<long double>(value); };
    contact_entity_count(entity);
    int index = 0;
    if (contact_entity_count(entity) > 0)
        do {
            void *nearby = contact_entity(entity, index);
            void *racer = invoke<void *>(entities::interface_at(nearby, 2), 2);
            if (racer) {
                const float distance =
                    float(static_cast<long double>(ai::pair_distance(entity, nearby)));
                const float speed =
                    float(static_cast<long double>(invoke<long double>(racer, 22)) *
                          float_word(1074735832));
                if (invoke<Byte>(racer, 13) && !recently_stunned(racer, 0)) {
                    void *race = at<void *>(system(92), 4);
                    bool immune = false;
                    if (invoke<Byte>(race, 45, network::world::vehicle_slot(racer))) {
                        void *player = invoke<void *>(entities::interface_at(racer, 2), 1);
                        if (player) {
                            const auto limit =
                                E(at<float>(at<void *>(player, 368), 1856)) *
                                static_cast<long double>(int(finish_immunity_seconds));
                            immune = E(at<float>(racer, 564)) < limit;
                        }
                    }
                    if (!immune) {
                        auto radius = E(speed) * at<float>(system(76), 84564);
                        if (!(E(minimum_detection_radius) < radius))
                            radius = E(minimum_detection_radius);
                        if (!(radius < E(distance)) &&
                            (speed > speeder_speed_mph || speed < reverse_speed_mph ||
                             at<Byte>(at<void *>(racer, 368), 7952))) {
                            lifecycle::dispatch_cop_event(static_cast<Dword *>(entity), 5);
                            invoke(entity, 57, racer, Dword(0));
                            break;
                        }
                    }
                }
            }
            ++index;
        } while (index < contact_entity_count(entity));
    return network_chase(entity) != nullptr;
}
}

namespace nfs::game::runtime::ai::cars::police_frame {
using namespace lifecycle::pending;
void update_cop_patrol_target_retained(void *entity, Dword controls) {

    using namespace lifecycle::pending;
    const Dword initial_tick = now();
    if (!at<Dword>(entity, 568))
        at<Dword>(entity, 568) = initial_tick;
    const Dword elapsed = initial_tick - at<Dword>(entity, 568);
    invoke(entity, 32, controls);
    bool respawn;
    void *target = at<void *>(entity, 1812);
    if (!target || !at<void *>(target, 368))
        respawn = elapsed >= lifecycle::state<Dword>(7352420);
    else {
        const Dword target_sample = at<Dword>(at<void *>(target, 368), 7984);
        const Dword own_sample = at<Dword>(at<void *>(entity, 368), 7984);
        void *view = entities::interface_at(target, 3);
        void *body = invoke<void *>(view, 0);
        if (body) {
            const Dword x = at<Dword>(body, 680), y = at<Dword>(body, 684);
            at<float>(view, 12) = -at<float>(body, 688);
            at<Dword>(view, 4) = x;
            at<Dword>(view, 8) = y;
        } else
            std::memcpy(bytes(view) + 4, runtime::saved::zero_vector, 12);
        if (at<Byte>(entity, 1840))
            at<Byte>(entity, 1840) = 0;
        Dword sample_delta = own_sample - target_sample;
        if (int(sample_delta) < 0)
            sample_delta += Dword(track_point_count(runtime::track_collision));
        const float distance = float(ai::pair_distance(entity, target));
        invoke<long double>(target, 22);
        if (at<float>(entity, 1820) < 1.f) {
            at<float>(entity, 1828) = distance;
            at<float>(entity, 1824) = distance;
            at<float>(entity, 1820) = distance;
        } else if (distance > at<float>(entity, 1824)) {
            at<float>(entity, 1824) = distance;
            ++at<Dword>(entity, 1836);
            at<Dword>(entity, 1832) = 0;
        } else {
            ++at<Dword>(entity, 1832);
            if (distance < at<float>(entity, 1828)) {
                at<Dword>(entity, 1836) = 0;
                at<float>(entity, 1828) = at<float>(entity, 1824) = distance;
            }
        }
        const bool too_far = distance > spawn_distance;
        const bool passed =
            at<int>(entity, 1844) != -1 && int(sample_delta) > at<int>(entity, 1844);
        at<Dword>(entity, 1844) = sample_delta;
        respawn = passed || (too_far && elapsed >= lifecycle::state<Dword>(7352420));
    }
    if (respawn) {
        const float range = spawn_distance;
        const float *position = invoke<const float *>(entity, 0);
        const Byte nearby = lifecycle::player_within_range(system(76), position, range);
        if (lifecycle::state<Dword>(7352732) || !nearby) {
            lifecycle::spawn_path_vehicle(lifecycle::state<Dword *>(7352448),
                                          static_cast<Dword *>(entity));
            return;
        }
        at<Dword>(entity, 568) = now();
    }
    detect_speeding_racers(entity);
}
}

namespace nfs::game::runtime::ai::cars::police_frame {
using namespace lifecycle::pending;
void __fastcall autonomous_cop_recover_body_retained(void *entity, void *, Dword controls_word) {

    using namespace lifecycle::pending;
    const Dword delay = at<Dword>(entity, 372);
    if (int(delay) > 0 && at<int>(entity, 376) < int(now() - delay) && !network_chase(entity)) {
        const float range = spawn_distance;
        const float *position = invoke<const float *>(entity, 0);
        const Byte nearby = lifecycle::player_within_range(system(76), position, range);
        if (lifecycle::state<Dword>(7352732) || !nearby) {
            at<Dword>(entity, 376) = at<Dword>(entity, 372) = Dword(-1);
            if (at<Byte>(entity, 576))
                lifecycle::dispatch_cop_event(static_cast<Dword *>(entity), 3);
            else {
                release_cop_reservation(lifecycle::state<void *>(7352448), entity);
                lifecycle::reset_cop_vehicle(lifecycle::state<Dword *>(7352448),
                                             static_cast<Dword *>(entity));
            }
        }
    }
    const int mode = at<int>(entity, 572);
    if (mode == 1)
        lifecycle::dispatch_cop_event(static_cast<Dword *>(entity), 11);
    else if (mode == 2)
        update_cop_patrol_target(entity, controls_word);
    else if (mode == 7 && !at<Byte>(entity, 576)) {
        if (!network_chase(entity))
            detect_speeding_racers(entity);
        if (!network_chase(entity)) {
            void *path = bytes(entity) + 220;
            int sample;
            Dword node;
            entities::pending::path_frame::advance_distance(path, 2.f, &sample, &node);
            const auto *point = reinterpret_cast<const float *>(node);
            float target[3] = {point[0], point[1], point[2]}, pose[21];
            lifecycle::sample_path(path, sample, pose);
            if (at<int>(entity, 240) == -1)
                dependencies::reverse_section_pose(pose);
            const float width = path_side_width(path, point);
            float offset[3];
            runtime::scale_vectors(1, pose + 9, width, offset);
            for (unsigned axis = 0; axis < 3; ++axis)
                target[axis] = float(static_cast<long double>(offset[axis]) + target[axis]);
            police_math::steer_car(entity, target, reinterpret_cast<void *>(controls_word));
            if (police_math::lateral_above_three_quarters(entity))
                invoke(entity, 32, controls_word);
            else
                invoke(entity, 33, Dword(1101004800), controls_word);
        }
    }
}
}

namespace nfs::game::runtime::ai::cars::police_frame {
using namespace lifecycle::pending;
Byte relocate_distant_chase_cop_retained(void *entity) {

    void *target = chased_entity(entity);
    if (invoke<Dword>(entities::interface_at(target, 1), 0) != 1)
        return 0;
    if (!police_math::heading_allows_relocation(target))
        return 0;
    const float *own_position = invoke<const float *>(entity, 0);
    const auto unseen = police_math::local_player_distance(own_position);
    if (unseen <= police_math::number(250))
        return 0;
    const auto target_distance = static_cast<long double>(ai::pair_distance(entity, target));
    const float stored_distance = float(target_distance);
    void *settings = bytes(system(76)) + 84520;
    if (!(police_math::number(at<float>(settings, 0)) < target_distance))
        return 0;
    void *path = bytes(target) + 220;
    const int minimum = path_minimum_sample(path, 0);
    float candidate[3];
    std::memcpy(candidate, invoke<const float *>(target, 0), 12);
    int index = -1;
    float distance = 0;
    float pose[21];
    if (minimum < 0) {
        do {
            lifecycle::pending::path_position(path, index);
            lifecycle::sample_path(path, index, pose);
            for (unsigned axis = 0; axis < 3; ++axis)
                candidate[axis] = float(police_math::number(pose[18 + axis]) + pose[axis]);
            const float *position = invoke<const float *>(target, 0);
            const auto separation = police_math::point_distance(candidate, position);
            distance = float(separation);
            if (police_math::number(at<float>(settings, 4)) < separation &&
                !(police_math::local_player_distance(candidate) <= police_math::number(250)))
                break;
            index = int(Dword(index) - 1);
        } while (minimum <= index);
    }
    if (distance > stored_distance ||
        police_math::local_player_distance(candidate) <= police_math::number(250))
        return 0;
    candidate[2] = -candidate[2];
    alignas(4) Byte ground[84];
    physics::clear_ground_contact(ground);
    bool supported = physics::sample_ground(candidate, ground) != 0;
    if (!supported) {
        void *compartment = track::boundary(system(36), 1, 0);
        if (compartment && world::find_supported_spawn(compartment, candidate)) {
            float visible[3] = {candidate[0], candidate[1], -candidate[2]};
            supported = !(police_math::local_player_distance(visible) <= police_math::number(250));
        }
    }
    if (!supported)
        return 0;
    lifecycle::sample_path(path, int(Dword(index) + 1), pose);
    const void *sample = lifecycle::pending::path_position(path, index);
    float maximum_speed = at<float>(sample, 24);
    if (!(entities::pending::minimum_path_width < maximum_speed))
        maximum_speed = entities::pending::minimum_path_width;
    float heading[3] = {float(police_math::number(pose[18]) + pose[0] - candidate[0]), 0,
                        float(police_math::number(pose[20]) + pose[2] + candidate[2])};
    if (heading[0] != 0 || heading[2] != 0)
        normalize_vector(heading, heading);
    else {
        float previous[21];
        lifecycle::sample_path(path, index, previous);
        std::memcpy(heading, previous + 9, 12);
    }
    float rotation[9] = {-heading[2], 0, -heading[0], 0, 1, 0, heading[0], 0, -heading[2]};
    lifecycle::pending::position_entity(entity, candidate, rotation, 0);
    const auto speed = static_cast<long double>(invoke<long double>(target, 22)) *
                       1.2f;
    float selected = float(speed);
    if (speed < police_math::number(6.70545f))
        selected = 6.70545f;
    else if (maximum_speed < selected)
        selected = maximum_speed;
    lifecycle::pending::set_car_speed(at<void *>(entity, 368), selected);
    at<float>(entity, 392) = 1;
    return 1;
}
}

namespace nfs::game::runtime::ai::cars::police_frame {
using namespace lifecycle::pending;
void __fastcall ai_cop_frame_retained(void *entity, void *) {

    if (at<Byte>(entity, 212)) {
        auto E = [](float value) { return static_cast<long double>(value); };
        const float progress = at<float>(entity, 564);
        void *population = system(76);
        auto difference = E(-1000000.f);
        if (!(progress < 0.f)) {
            const float target_progress = at<float>(at<void *>(entity, 664), 584);
            if (!(target_progress < 0.f)) {
                const float delta = float(E(target_progress) - progress);
                const Dword group = invoke<Dword>(entity, 15);
                const float length = at<float>(population, group * 42080 + 16868);
                difference = E(delta);
                if (difference < E(length) * -0.5f)
                    difference = difference + length;
                else if (E(length) * 0.5f < difference)
                    difference = difference - length;
            }
        }
        const auto missing_progress = E(-1000000.f);
        if (difference < missing_progress || missing_progress < difference ||
            std::isnan(difference)) {
            if (difference < E(0))
                difference = -difference;
            const float threshold = at<float>(population, 84572);
            if (!std::isnan(difference) && !std::isnan(threshold) &&
                !(difference < E(threshold))) {
                const float *position = invoke<const float *>(entity, 0);
                if (!lifecycle::player_within_range(system(76), position, 300.f)) {
                    const float range = at<float>(system(76), 84576);
                    const float player_range = range < 300.f ? range : 300.f;
                    Dword route;
                    std::memcpy(&route, &player_range, 4);
                    int sample;
                    std::memcpy(&sample, &range, 4);
                    if (lifecycle::pending::find_pursuit_spawn(at<void *>(entity, 664), 0, range,
                                                               player_range, &route, &sample)) {
                        float spawn[3], rotation[9];
                        lifecycle::pending::sample_route_pose(reinterpret_cast<void *>(route),
                                                              sample, spawn, rotation, 0);
                        lifecycle::pending::position_entity(entity, spawn, rotation, 0);
                        player_frame(entity, nullptr);
                        return;
                    }
                }
            }
        }
    }
    player_frame(entity, nullptr);
}
}

namespace nfs::game::runtime::ai::cars::police_frame {
using namespace lifecycle::pending;
Dword __fastcall player_pursuit_event_retained(void *entity, void *, Dword controls_word) {

    if (at<float>(entity, 636) > 1.f) {
        at<Dword>(entity, 632) = 900;
        at<float>(entity, 636) = 0.f;
    }
    if (at<int>(entity, 632) < 1 || !at<void *>(entity, 628))
        return 0;
    const float *target_position = invoke<const float *>(at<void *>(entity, 628), 0);
    const float *own_position = invoke<const float *>(entity, 0);
    if (static_cast<long double>(80.f) <
        police_math::point_distance(own_position, target_position))
        return 0;
    void *target = at<void *>(entity, 628);
    if (!target_on_neighboring_section(entity, target))
        return 0;
    --at<Dword>(entity, 632);
    const float own_progress = float(ai::race_progress_x87::priority(entity));
    if (ai::race_progress_x87::priority(target) + 10.f <
        static_cast<long double>(own_progress))
        return 0;
    police_math::steer_car(entity, invoke<const float *>(target, 0),
                          reinterpret_cast<void *>(controls_word));
    const auto speed =
        static_cast<long double>(invoke<long double>(target, 22)) * 1.2f;
    // Unordered speed chooses the saved minimum, just as TEST AH,41 / JNE.
    const float requested =
        static_cast<long double>(float_word(1108282547)) < speed ? float(speed) : float_word(1108282547);
    invoke(entity, 33, requested, controls_word);
    if (at<int>(entity, 632) % 150 == 0)
        select_contact_voice(entity, "ZDZ");
    return 1;
}
}

namespace nfs::game::runtime::ai::cars::police_frame {
using namespace lifecycle::pending;
void __fastcall player_path_brake_retained(void *entity, void *, Dword controls_word) {

    auto E = [](float value) { return static_cast<long double>(value); };
    void *body = invoke<void *>(entities::interface_at(entity, 3), 0);
    if ((!body || !at<Byte>(body, 728)) && invoke<long double>(entity, 22) > 4.470300197601318359375f) {
        const auto speed =
            static_cast<long double>(invoke<long double>(entity, 22)) * 0.97f;
        invoke(entity, 33, float(speed), controls_word);
        return;
    }
    body = invoke<void *>(entities::interface_at(entity, 3), 0);
    if (body && at<Byte>(body, 728) && invoke<long double>(entity, 22) > 8.94060039520263671875f &&
        at<Byte>(entity, 596)) {
        float output[3] = {0.f, 0.f, 0.f};
        Dword point = 0;
        float width;
        const float *position = invoke<const float *>(entity, 0);
        const float distance = float(invoke<long double>(entity, 22));
        float pose[21];
        entities::pending::sample_path_target(bytes(entity) + 220, output, &width, &point, pose,
                                              distance, position, 0);
        const unsigned edge = at<Byte>(entity, 576) & 1 ? 15 : 12;
        float target[3];
        for (unsigned axis = 0; axis < 3; ++axis)
            target[axis] = float(E(pose[edge + axis]) + pose[axis]);
        invoke(entity, 54, target, target, width, &width, 6.f);
        entities::brake_path_entity(entity, nullptr, reinterpret_cast<Dword *>(controls_word));
        police_math::steer_car(entity, target, reinterpret_cast<void *>(controls_word));
        return;
    }
    entities::brake_path_entity(entity, nullptr, reinterpret_cast<Dword *>(controls_word));
}
}

namespace nfs::game::runtime::ai::lifecycle::police_frame {
Byte assign_pursuer_retained(void *entity, void *target) {

    if (at<Dword>(entity, 568))
        return 0;
    // The exceptional legacy __ftol range remains an explicit numerical boundary.
    const Dword delay = police_math::truncate(static_cast<long double>(at<float>(entity, 572)) * -60.0f);
    const Dword deadline = at<Dword>(entity, 564) - delay;
    if (static_cast<int>(graphics::unresolved::camera_state) < static_cast<int>(deadline))
        return 0;
    void *car = at<void *>(entity, 368);
    void *path = bytes(target) + 220;
    float selected_pose[21];
    bool found = false;
    for (int distance = 10; distance < path_length(path, 0); distance += 10) {
        void *section = path_section_at(path, distance);
        if (!section || at<int>(section, 32) > 1)
            continue;
        float pose[21];
        sample_path(path, distance, pose);
        const float ground_position[3] = {pose[0], pose[1], -pose[2]};
        if (!physics::pending::sample_ground_entry(ground_position,
                                                      bytes(at<void *>(car, 1768)) + 508))
            break;
        std::memcpy(selected_pose, pose, sizeof(pose));
        found = true;
    }
    if (!found)
        return 0;
    invoke(entity, 6, Dword{1});
    void *physics = at<void *>(car, 1768);
    at<Byte>(physics, 728) = at<Byte>(physics, 729) = 1;
    at<Dword>(entity, 568) = 1;
    void *own_path = bytes(entity) + 220;
    for (unsigned offset = 0; offset < 40; offset += 4)
        at<Dword>(own_path, offset) = at<Dword>(path, offset);
    copy_path_reference(bytes(own_path) + 40, bytes(path) + 40);
    float position[3] = {selected_pose[0], selected_pose[1], selected_pose[2]};
    if (selected_pose[3] == 0 && selected_pose[5] == 0)
        return 0;
    float orientation[9] = {selected_pose[3], 0, -selected_pose[5], 0, 1, 0,
                            selected_pose[5], 0, selected_pose[3]};
    matrices::orthonormalize(orientation);
    if (random_fraction() <= 0.75L) {
        at<Dword>(entity, 580) = 0u - at<Dword>(path, 20);
        rotate_y_basis(orientation, -0.25f, orientation);
        reverse_path(own_path);
    } else {
        at<Dword>(entity, 580) = at<Dword>(path, 20);
        rotate_y_basis(orientation, 0.25f, orientation);
    }
    position[2] = -position[2];
    position[1] += 1.0f;
    alignas(4) Byte reference[48];
    reset_path_reference(reference);
    Dword sample, direction;
    const Dword group = invoke<Dword>(entity, 15);
    pending::construct_pursuit_route(bytes(system(76)) + 4 + group * 42080, reference, &sample,
                                     &direction, selected_pose + 3, selected_pose, own_path, 1, 0);
    clear_path(own_path);
    bind_path(own_path, reference, sample, at<Dword>(entity, 580), entity);
    pending::position_entity(entity, position, orientation, 1);
    void *cursor = bytes(entity) + 236;
    if (path_current_section(cursor) && at<Dword>(path_current_section(cursor), 32) == 1) {
        pending::set_car_speed(car, 3.0f);
        at<Dword>(entity, 584) = 0;
        return 1;
    }
    invoke(entity, 6, Dword{0});
    physics = at<void *>(car, 1768);
    at<Byte>(physics, 728) = at<Byte>(physics, 729) = 0;
    at<Dword>(entity, 568) = 0;
    return 0;
}
}

namespace nfs::game::runtime::ai::lifecycle::pending::police_frame {
Byte find_pursuit_spawn_retained(void *entity, Byte reverse, float minimum_distance, float player_range,
                        Dword *section_output, int *node_output) {

    void *initial_section = invoke<void *>(entity, 1);
    int node = invoke<int>(entity, 2);
    Dword *population = system(76);
    const Dword group_index = invoke<Dword>(entity, 60 / 4);
    alignas(4) Byte reference[48];
    construct_path_reference(reference, bytes(population) + 4 + group_index * 42080,
                             at<Dword>(initial_section, 20 + (reverse ? 4 : 0)));
    select_path_section(reference, initial_section);
    float distance = 0;
    auto X = [&](float value) { return static_cast<long double>(value); };
    int section_budget = 100;
    for (;;) {
        void *section = at<void *>(reference, 8);
        const float progress = at<float>(dependencies::section_node(section, node), 28);
        const auto candidate =
            (reverse ? X(progress) : X(at<float>(section, 44)) - progress) + distance;
        if (!(candidate < X(minimum_distance)))
            break;
        distance = float(candidate);
        enumerate_path_sections(reference, 0);
        choose_least_used_section(reference);
        section = at<void *>(reference, 8);
        node = reverse ? int(at<Dword>(section, 36) - 1) : 0;
        if (--section_budget <= 0)
            break;
    }
    if (section_budget < 0)
        return 0;
    int node_budget = 1000;
    do {
        void *section = at<void *>(reference, 8);
        if (reverse ? node <= 0 : node >= int(at<Dword>(section, 36) - 1)) {
            enumerate_path_sections(reference, 0);
            choose_least_used_section(reference);
            section = at<void *>(reference, 8);
            node = reverse ? int(at<Dword>(section, 36) - 1) : 0;
        }
        const int next_node = reverse ? node - 1 : node + 1;
        // The sum is stored to binary32 before the second distance is subtracted.
        const float sum = float(
            X(at<float>(dependencies::section_node(section, reverse ? node : next_node), 28)) +
            distance);
        distance =
            float(X(sum) -
                  at<float>(dependencies::section_node(section, reverse ? next_node : node), 28));
        node = next_node;
        if (minimum_distance <= distance &&
            !player_within_range(
                system(76), static_cast<const float *>(dependencies::section_node(section, node)),
                player_range))
            break;
    } while (--node_budget > 0);
    *node_output = node;
    *section_output = at<Dword>(reference, 8);
    return 1;
}
}

namespace nfs::game::runtime::ai::lifecycle::pending::police_frame {
std::uint32_t chase_ticks(float seconds) {
    return police_math::truncate(static_cast<long double>(seconds) * 60.0f);
}
float evade_fraction(float percentage) {
    return float(static_cast<long double>(percentage) * 0.01f);
}
}
