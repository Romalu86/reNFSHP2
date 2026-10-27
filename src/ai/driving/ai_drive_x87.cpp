#include "../player/player_drive_runtime.hpp"
#include "../../network/session/network_pursuit_runtime.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include <cmath>
namespace nfs::application::pending { void *traffic_actor_view(void *); }
namespace nfs::game::runtime::ai::entities::pending { void notify_projectile_target(void *, Dword); }
#include "ai_driver_runtime.hpp"
#include "ai_drive_x87.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
namespace nfs::game::runtime::ai::cars {
namespace {
namespace path_speed_layout {
constexpr unsigned entity_car = 368;
constexpr unsigned driving_state = 600;
constexpr unsigned race_position = 576;
constexpr unsigned car_maximum_speed = 572;
constexpr unsigned car_grip = 576;
constexpr unsigned body_active = 728;
constexpr unsigned contact_surface = 532;
constexpr unsigned contact_material = 586;
constexpr unsigned surface_grip = 4;
constexpr unsigned body_interface = 3;
constexpr unsigned racer_profile_interface = 5;
constexpr unsigned race_tuning_system = 76;
constexpr unsigned race_rules_system = 92;
constexpr unsigned race_rules_controller = 4;
constexpr unsigned racer_count_method = 23;
constexpr unsigned grip_limit_method = 53;
constexpr unsigned glue_speed_factor_method = 58;
constexpr Dword starting_path_state = 3;
constexpr Byte material_index_mask = 31;
// racetune.ini: Racer.OverallSpeedFactor, Racer.SkillSpeedRange, Racer.ClassFactor.
constexpr unsigned overall_speed_factor = 84492;
constexpr unsigned skill_speed_range = 84488;
constexpr unsigned class_factor = 84496;
}

long double clamp_racer_factor(long double value) {
    if (value < 0.0L)
        return 0.0L;
    if (1.0L < value)
        return 1.0L;
    return value;
}
}

// Keep the virtual result in the native return register.
long double __fastcall car_grip_limit(void *entity, void *) {
    using namespace path_speed_layout;
    const Dword grip_bits = at<Dword>(at<void *>(entity, entity_car), car_grip);
    void *body = invoke<void *>(entities::interface_at(entity, body_interface), 0);
    long double result;
    if (body && at<Byte>(body, body_active)) {
        void *reference = physics::pending::surface_properties(1);
        body = invoke<void *>(entities::interface_at(entity, body_interface), 0);
        const Byte material = at<void *>(body, contact_surface)
                                  ? Byte(at<Byte>(body, contact_material) & material_index_mask)
                                  : Byte(0);
        void *properties = physics::pending::surface_properties(material);
        const long double ratio = static_cast<long double>(at<float>(properties, surface_grip)) /
                                  at<float>(reference, surface_grip);
        result = ratio < 1.0L ? ratio * float_word(grip_bits)
                             : static_cast<long double>(float_word(grip_bits));
    } else {
        result = static_cast<long double>(float_word(grip_bits));
    }
    if (!(result < 1.0L))
        result = 1.0L;
    return result;
}

namespace drive_frame {
namespace legacy {
int corridor_lane_native80(const void *projection) {
    long double retained_projection;
    std::memcpy(&retained_projection, projection, 10);
    const float shifted = float(retained_projection + 0.5f);
    const auto rounded = static_cast<long double>(compatibility_lane_floor(shifted));
    const Dword lane_bits = Dword(hardware::truncate_integer_native80(&rounded)) + 20u;
    std::int32_t lane;
    std::memcpy(&lane, &lane_bits, sizeof lane);
    return audio::vehicle::clamp_value(lane, 0, 39);
}
} // namespace legacy

namespace {
// Explicit float locals are the original stored intermediates.
long double calculate_player_path_target_speed(void *entity, float requested_speed) {
    using namespace path_speed_layout;
    constexpr float minimum_class_speed = 55.88f;
    constexpr float maximum_class_speed = 92.54f;
    constexpr float skill_step = 0.1f;
    void *car = at<void *>(entity, entity_car);
    if (services::pending::car_is_remote(car))
        return static_cast<long double>(requested_speed);

    const Dword overall_speed_bits = at<Dword>(system(race_tuning_system), overall_speed_factor);
    float overall_speed = float_word(overall_speed_bits);
    if (at<Dword>(entity, driving_state) == starting_path_state) {
        const int racer_count = invoke<int>(at<void *>(system(race_rules_system), race_rules_controller),
                                            racer_count_method);
        if (racer_count > 3) {
            const long double rank_fraction = static_cast<long double>(at<int>(entity, race_position)) /
                                              static_cast<long double>(racer_count);
            const long double speed_difference = 1.0L - overall_speed;
            overall_speed = float(1.0L - speed_difference * rank_fraction);
        }
    }

    void *tuning = system(race_tuning_system);
    const float skill_range = at<float>(tuning, skill_speed_range);
    const std::uint16_t skill = at<std::uint16_t>(entities::interface_at(entity, racer_profile_interface), 0);
    const long double skill_fraction = clamp_racer_factor(static_cast<long double>(skill) * skill_step);
    const float skill_scale = float(skill_fraction * skill_range + (1.0L - skill_range));
    float vehicle_class_scale = 1.0f;
    if (car) {
        const long double class_speed = static_cast<long double>(at<float>(car, car_maximum_speed)) -
                                        minimum_class_speed;
        const long double class_range = static_cast<long double>(maximum_class_speed) - minimum_class_speed;
        const long double class_fraction = clamp_racer_factor(class_speed / class_range);
        vehicle_class_scale = float(1.0L - (1.0L - class_fraction) * at<float>(tuning, class_factor));
    }

    const float race_position_scale = float(invoke<long double>(entity, glue_speed_factor_method));
    const float maximum_speed = at<float>(car, car_maximum_speed);
    long double grip_scale = 1.0L;
    if (maximum_speed != 0.0f) {
        if (maximum_speed < requested_speed) {
            requested_speed = maximum_speed;
        } else {
            const long double grip_response = invoke<long double>(entity, grip_limit_method);
            const long double missing_grip = 1.0L - grip_response;
            const long double requested_fraction = static_cast<long double>(requested_speed) / maximum_speed;
            grip_scale = 1.0L - (1.0L - requested_fraction) * missing_grip;
        }
    }

    const long double ranked_speed = static_cast<long double>(overall_speed) * race_position_scale;
    const long double class_speed = ranked_speed * vehicle_class_scale;
    const long double gripped_speed = class_speed * grip_scale;
    long double speed_scale = gripped_speed * skill_scale;
    if (!(speed_scale < 1.0L))
        speed_scale = 1.0L;
    return speed_scale * requested_speed;
}
}

void player_target_speed_native80(void *entity, float requested_speed, void *output) {
    const long double speed = calculate_player_path_target_speed(entity, requested_speed);
    std::memcpy(output, &speed, 10);
}

float player_target_speed_f32(void *entity, float requested_speed) {
    return float(calculate_player_path_target_speed(entity, requested_speed));
}
} // namespace drive_frame
} // namespace nfs::game::runtime::ai::cars

namespace nfs::game::runtime::ai::cars::drive_frame {
void traffic_controls(void *entity, Dword controls_word) {
    if (!at<Dword>(entity, 568))
        return;
    auto E = [](float value) { return static_cast<long double>(value); };
    Dword point = 0;
    auto speed = static_cast<long double>(invoke<long double>(entity, 22));
    if (speed < E(0))
        speed = -speed;
    if (speed < E(float_word(1091505331)))
        speed = E(float_word(1091505331));
    const float distance = float(speed);
    const float *position = invoke<const float *>(entity, 0);
    float output[3], pose[21], width;
    void *path = bytes(entity) + 220;
    if (!entities::pending::sample_path_target(path, output, &width, &point, pose, distance,
                                               position, 0))
        return;
    float target[3];
    std::memcpy(target, output, 12);
    Dword desired_word = at<Dword>(entity, 576);
    if (traffic_yields_to_chase(entity)) {
        const float direction = at<int>(entity, 240) > 0 ? 1.f : -1.f;
        desired_word = 1087804172;
        for (unsigned axis = 0; axis < 3; ++axis)
            target[axis] = float(E(pose[15 + axis]) * direction + pose[axis]);
        if (legacy::lateral_beyond_three_quarters(entity))
            desired_word = 0;
    } else {
        bool steer_path = !entities::path_has_section(path);
        if (!steer_path) {
            void *car = at<void *>(entity, 368);
            if (at<std::uint16_t>(at<void *>(car, 1768), 596) <= 32) {
                float current_pose[21], forward[3];
                lifecycle::sample_path(path, 0, current_pose);
                current_pose[5] = -current_pose[5];
                std::memcpy(forward, bytes(at<void *>(car, 1768)) + 716, 12);
                const auto along = hardware::dot_product(forward, current_pose + 3);
                const float alignment =
                    float(along * static_cast<long double>(at<int>(entity, 240)));
                const long double current_speed = invoke<long double>(entity, 22);
                steer_path = (current_speed > float_word(1087804172) && alignment > 0.5f) ||
                             !(alignment < 0.5f);
            }
            if (!steer_path) {
                if (invoke<Byte>(entity, 45, controls_word, float_word(1087804172)))
                    return;
            }
        }
        if (steer_path) {
            const unsigned edge = at<int>(entity, 240) > 0 ? 15 : 12;
            for (unsigned axis = 0; axis < 3; ++axis)
                target[axis] = float(E(pose[edge + axis]) * 0.5f + pose[axis]);
            const Byte clear = invoke<Byte>(entity, 54, target, target, at<Dword>(entity, 576),
                                            &desired_word, 10.f);
            if (!clear && traffic_vehicle_ahead(entity))
                desired_word = 0;
        }
    }
    discard_steering(entity, target, reinterpret_cast<void *>(controls_word));
    invoke(entity, 33, desired_word, controls_word);
}
}

namespace nfs::game::runtime::ai::cars::drive_frame {
using namespace lifecycle::pending;
Byte traffic_ahead(void *entity) {
    for (int index = 0; index < at<int>(entity, 8); ++index) {
        void *other = at<void *>(entity, 12 + Dword(index) * 4);
        void *player = invoke<void *>(entities::interface_at(other, 2), 1);
        if (!player || !target_on_neighboring_section(entity, player))
            continue;
        const float *other_position = invoke<const float *>(player, 0);
        const float *position = invoke<const float *>(entity, 0);
        if (static_cast<long double>(50.f) < drive_frame::point_distance(position, other_position))
            break;
        void *body = invoke<void *>(entities::interface_at(entity, 3), 0);
        void *other_body = invoke<void *>(entities::interface_at(player, 3), 1);
        float delta[3], relative[3] = {0.f, 0.f, 0.f};
        for (unsigned axis = 0; axis < 3; ++axis)
            delta[axis] = float(static_cast<long double>(at<float>(other_body, 680 + axis * 4)) -
                                at<float>(body, 680 + axis * 4));
        matrices::transform_vectors(1, delta, reinterpret_cast<const float *>(bytes(body) + 72),
                                    relative);
        if (relative[2] < 30.f && relative[2] > at<float>(body, 128)) {
            const float lateral = relative[0] < 0.f ? -relative[0] : relative[0];
            const auto width = static_cast<long double>(at<float>(body, 120)) + at<float>(body, 120);
            if (static_cast<long double>(lateral) < width)
                return 1;
        }
    }
    return 0;
}

void racer_body_contact(void *entity, Dword contact_word) {
    void *contact = reinterpret_cast<void *>(contact_word);
    if (at<Dword>(contact, 48) == 2 && at<void *>(contact, 0)) {
        void *car = physics::vehicle_car(at<void *>(contact, 0));
        void *other = car ? at<void *>(car, 7988) : nullptr;
        if (other) {
            void *cop = application::pending::traffic_actor_view(other);
            if (cop && at<Byte>(cop, 576)) {
                const auto strength = static_cast<long double>(at<float>(contact, 4)) /
                                      static_cast<long double>(at<float>(contact, 56));
                if (static_cast<long double>(float_word(1091505331)) < strength && !at<Byte>(entity, 600))
                    entities::pending::notify_projectile_target(entity, 4);
            }
        }
    }
    cars::racer_body_contact(entity, nullptr, contact_word);
}

Dword accept_player_side_contact(void *entity, Dword other_word, Dword speed_word) {
    void *other = reinterpret_cast<void *>(other_word);
    if (car_is_traffic(other)) {
        car_accept_side_contact(entity, nullptr, other_word, speed_word);
        return 0;
    }
    auto E = [](float value) { return static_cast<long double>(value); };
    const float other_speed = float(invoke<long double>(at<void *>(other, 7988), 22));
    auto difference =
        E(other_speed) - static_cast<long double>(invoke<long double>(entity, 22));
    if (difference < E(0))
        difference = -difference;
    const float tolerance = float_word(1087804172);
    if (!(difference < E(tolerance)))
        return 0;
    const auto current_speed =
        static_cast<long double>(invoke<long double>(entity, 22));
    const auto threshold = E(at<float>(entity, 624)) - tolerance;
    const bool ordered =
        !std::isnan(threshold) && !std::isnan(current_speed);
    if (!(ordered && !(current_speed < threshold)) && at<Dword>(entity, 600) != 3)
        return 0;
    if (audio::vehicle::car_audio_suppressed(other))
        car_accept_side_contact(entity, nullptr, other_word, speed_word);
    return 1;
}

void player_before_start(void *entity, Dword controls) {
    Dword point = 0;
    float position[3], pose[21], width;
    const float *current = invoke<const float *>(entity, 0);
    if (entities::pending::sample_path_target(bytes(entity) + 220, position, &width, &point, pose,
                                              20.f, current, 0) &&
        std::int32_t(start_deadline - graphics::unresolved::camera_state) < 30)
        invoke(entity, 33, 40.f, controls);
}

void player_start_path(void *entity, Dword controls) {
    Dword point = 0;
    long double speed = invoke<long double>(entity, 22);
    if (speed < 0)
        speed = -speed;
    const float distance = speed < float_word(1091505331) ? float_word(1091505331) : float(speed);
    float position[3], pose[21], width;
    const float *current = invoke<const float *>(entity, 0);
    if (!entities::pending::sample_path_target(bytes(entity) + 220, position, &width, &point, pose,
                                               distance, current, 0))
        return;
    const float requested = player_target_speed_f32(entity, width);
    at<float>(entity, 624) = requested;
    invoke(entity, 54, position, position, requested, bytes(entity) + 624, 5.f);
    invoke(entity, 33, at<Dword>(entity, 624), controls);
    discard_steering(entity, position, reinterpret_cast<void *>(controls));
}

void player_path(void *entity, Dword controls) {
    auto E = [](float value) { return static_cast<long double>(value); };
    auto speed = [&] {
        return static_cast<long double>(invoke<long double>(entity, 22));
    };
    void *body = invoke<void *>(entities::interface_at(entity, 3), 0);
    if (!body || !at<Byte>(body, 728)) {
        Dword point = 0;
        float position[3], pose[21], width;
        const float *current = invoke<const float *>(entity, 0);
        if (entities::pending::sample_path_target(bytes(entity) + 220, position, &width, &point,
                                                  pose, 10.f, current, 0)) {
            const float requested = player_target_speed_f32(entity, width);
            at<float>(entity, 624) = requested;
            invoke(entity, 33, requested, controls);

            discard_steering(entity, reinterpret_cast<const float *>(point),
                                  reinterpret_cast<void *>(controls));
        }
        return;
    }
    if (invoke<Byte>(entity, 57, controls))
        return;
    if (invoke<Dword>(entity, 50)) {
        int period;
        switch (at<Dword>(entities::interface_at(entity, 5), 8)) {
        case 1:
            period = 16;
            break;
        case 2:
            period = 12;
            break;
        case 4:
            period = 6;
            break;
        case 5:
            period = 4;
            break;
        default:
            period = 8;
            break;
        }
        if (invoke<Dword>(entities::interface_at(entity, 1), 0) == 5)
            period += 4;
        if (period &&
            std::int32_t(graphics::unresolved::camera_state) % period < period / 2) {
            auto steering = E(at<float>(at<void *>(entity, 368), 7992)) * 127.f;
            // The native carry predicate selects this branch for unordered values too.
            steering = !(E(0) <= steering) ? steering + E(float_word(1056964574))
                                          : steering - E(float_word(1056964574));
            const float rounded_source = float(steering);
            const int result = hardware::current_integer_f32(rounded_source);
            at<Byte>(reinterpret_cast<void *>(controls), 4) = Byte(result);
            const float requested = float(invoke<long double>(entity, 22));
            invoke(entity, 33, requested, controls);
            return;
        }
    }
    void *car = at<void *>(entity, 368);
    auto magnitude = speed();
    if (magnitude < E(0))
        magnitude = -magnitude;
    if (magnitude < E(float_word(1091505331)))
        magnitude = E(float_word(1091505331));
    Dword point = 0;
    float position[3], target[3], pose[21];
    auto *requested = reinterpret_cast<float *>(bytes(entity) + 624);
    const float *current = invoke<const float *>(entity, 0);
    if (!entities::pending::sample_path_target(bytes(entity) + 220, position, requested, &point,
                                               pose, float(magnitude), current, 0))
        return;
    float width = *requested;
    if (!(width > float_word(1091505331)))
        width = float_word(1091505331);
    *requested = width;
    width = player_target_speed_f32(entity, width);
    *requested = width;
    invoke(entity, 54, position, target, width, requested, 5.f);
    discard_steering(entity, target, reinterpret_cast<void *>(controls));
    auto slip = E(at<float>(car, 7992)) * at<float>(car, 124);
    if (slip < E(0))
        slip = -slip;
    const float stored_slip = float(slip);
    const float difference = float(E(*requested) - speed());
    if (stored_slip > float_word(1023969417) && E(float_word(1104581388)) < speed()) {
        auto ratio = (E(stored_slip) - E(float_word(1023969417))) /
                     (E(at<float>(car, 124)) - E(float_word(1023969417)));
        if (ratio < E(0))
            ratio = E(0);
        else if (E(1) < ratio)
            ratio = E(1);
        const float blend = float(ratio);
        *requested = float(speed() + (E(1) - E(blend)) * difference);
    }
    invoke(entity, 33, *requested, controls);
}

void toggle_pursuit(void *entity) {
    auto E = [](float value) { return static_cast<long double>(value); };
    if (!race_started)
        return;
    if (at<Byte>(at<void *>(entity, 368), 1579)) {
        if (!network_chase(entity))
            at<Byte>(at<void *>(entity, 368), 1579) = 0;
        return;
    }
    if (network_chase(entity) || legacy::route_heading_negative(entity))
        return;
    float heading[3];
    void *selected = nullptr;
    invoke(entities::interface_at(entity, 3), 2, heading);
    void *track = system(76);
    const Dword route = invoke<Dword>(entity, 15);
    const float length = at<float>(track, route * 42080 + 16868);
    const int count = at<int>(system(76), 84172);
    float selected_distance;
    for (int index = 0; index < count; ++index) {
        void *other = at<void *>(system(76), 84176 + index * 4);
        void *cop = ai::police(other);
        if (cop && cop_unavailable(cop))
            continue;
        if (network_chase(other))
            continue;
        const auto retained_distance =
            static_cast<long double>(ai::pair_distance(entity, other));
        const float distance = float(retained_distance);
        if (E(500) < retained_distance)
            continue;
        auto delta_progress = E(at<float>(entity, 584)) - at<float>(other, 564);
        if (delta_progress < E(length) * -.5f)
            delta_progress = delta_progress + length;
        else if (E(length) * .5f < delta_progress)
            delta_progress = delta_progress - length;
        if (delta_progress < E(0))
            delta_progress = -delta_progress;
        if (E(500) < delta_progress)
            continue;
        const float *own_position = invoke<const float *>(entity, 0);
        const float *other_position = invoke<const float *>(other, 0);
        float direction[3];
        for (unsigned axis = 0; axis < 3; ++axis)
            direction[axis] = float(E(other_position[axis]) - own_position[axis]);
        if (direction[0] != 0.f || direction[1] != 0.f || direction[2] != 0.f)
            runtime::normalize_vector(direction, direction);
        const auto alignment = hardware::dot_product(direction, heading);
        if (!(alignment < E(.707f)) && (!selected || distance < selected_distance)) {
            selected = other;
            selected_distance = distance;
        }
    }
    if (selected) {
        at<Byte>(at<void *>(entity, 368), 1579) = 1;
        invoke(entity, 57, selected, Dword(0));
    }
}
}
