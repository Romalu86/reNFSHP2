#include "cop_driving_runtime.hpp"
#include "../../ai/driving/ai_car_behavior.hpp"
#include "../../ai/driving/ai_drive_x87.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include <cstring>
namespace nfs::game::runtime::ai::cars::drive_frame {
static long double calculate_route_heading(void *entity) {
    void *section = invoke<void *>(entity, 1);
    const int sample = invoke<int>(entity, 2);
    if (!section || sample < 0)
        return 1.0L;
    float tangent[3], local[3];
    dependencies::sample_horizontal_tangent(section, sample, tangent);
    if (at<int>(entity, 240) == -1)
        tangent[0] = -tangent[0];
    else
        tangent[2] = -tangent[2];
    void *body = invoke<void *>(entities::interface_at(entity, 3), 0);
    matrices::transform_vectors(1, tangent, reinterpret_cast<const float *>(bytes(body) + 72), local);
    return runtime::turn_direction_retained(local[0], local[2]);
}
void route_heading_native80(void *entity, void *output) {
    const long double result = calculate_route_heading(entity);
    std::memcpy(output, &result, 10);
}
}

namespace nfs::game::runtime::ai::cars::drive_frame {
Byte pursuit_controls(void *entity, Dword *controls, float speed_bias) {
    void *chase = network_chase(entity);
    if (!chase)
        return 0;
    int direction;
    float gap;
    Dword route_sample;
    if (!plan_cop_pursuit_route(entity, &direction, &gap, &route_sample))
        return 0;
    void *target = chased_entity(entity);
    if (!target)
        return 0;
    void *section = invoke<void *>(entity, 1);
    const int sample = invoke<int>(entity, 2);
    void *target_section = invoke<void *>(target, 1);
    const int target_sample = invoke<int>(target, 2);
    float tangent[3];
    dependencies::sample_horizontal_tangent(target_section, target_sample, tangent);
    void *target_body = at<void *>(at<void *>(target, 368), 1768);
    const float target_projection =
        float((number(at<float>(entity, 1848)) * 3.f + at<float>(target_body, 8)) * tangent[0] +
              (-number(at<float>(target_body, 16)) - number(at<float>(entity, 1852)) * 3.f) *
                  tangent[2]);
    float target_speed = float(magnitude(number(at<float>(at<void *>(target, 368), 3132))));
    int target_direction = 1;
    if (target_projection < 0) {
        target_speed = -target_speed;
        target_direction = -1;
    }
    dependencies::sample_horizontal_tangent(section, sample, tangent);
    void *body_view = invoke<void *>(entities::interface_at(entity, 3), 1);
    const float local_x = float(number(tangent[0]) * at<float>(body_view, 8));
    body_view = invoke<void *>(entities::interface_at(entity, 3), 1);
    const auto projection = number(local_x) - number(tangent[2]) * at<float>(body_view, 16);
    auto own_speed = magnitude(number(at<float>(at<void *>(entity, 368), 3132)));
    int own_direction = 1;
    if (projection < number(0)) {
        own_speed = -own_speed;
        own_direction = -1;
    }
    int matching_direction = direction;
    if (section != target_section) {
        const std::uint16_t count = at<std::uint16_t>(entity, 1800);
        if (count) {
            const std::uint16_t index = std::uint16_t(count - 1);
            section = index < count ? at<void *>(entity, 600 + unsigned(index) * 4) : nullptr;
        }
        matching_direction = at<Dword>(target_section, 24) == at<Dword>(section, 20) ? -1 : 1;
    }
    bool opposite_target = false, reverse_own_motion = false;
    // TEST AH,1 accepts ordered greater/equal and rejects unordered.
    if (matching_direction != target_direction &&
        !(number(target_speed) * 10.f < number(gap)) && !std::isnan(target_speed) &&
        !std::isnan(gap)) {
        direction = target_direction;
        opposite_target = true;
    }
    if (direction != own_direction && number(13.410900115966796875f) < own_speed) {
        direction = own_direction;
        reverse_own_motion = true;
    }
    if (at<Dword>(chase, 88) == 1)
        direction = target_direction;
    at<int>(entity, 1808) = direction;
    if (direction != at<int>(entity, 240))
        entities::pending::reverse_entity_path(entity);
    float pose[21];
    lifecycle::sample_path(bytes(entity) + 220, 0, pose);
    if (at<int>(entity, 240) == -1)
        dependencies::reverse_section_pose(pose);
    chase = network_chase(entity);
    const auto chase_fraction = integer(at<int>(chase, 72)) / integer(at<int>(chase, 68));
    bool use_target = false;
    float longitudinal_offset = 0, lateral_offset = 0, lateral_gain = 1, gap_gain = 1;
    float target_point[3];
    bool attempt_tactic = !(chase_fraction < number(.1f) && at<Dword>(entity, 580) == 7);
    void *population = nullptr;
    if (attempt_tactic) {
        population = system(76);
        void *current_target = chased_entity(entity);
        const auto distance = static_cast<long double>(ai::pair_distance(entity, current_target));
        attempt_tactic = !(number(at<float>(population, 84536)) < distance);
    }
    if (attempt_tactic) {
        void *current_target = chased_entity(entity);
        const float first_speed = float(invoke<long double>(current_target, 22));
        const auto speed_difference = magnitude(
            number(first_speed) - static_cast<long double>(invoke<long double>(entity, 22)));
        const float stored_difference = float(speed_difference);
        current_target = chased_entity(entity);
        const float second_speed = float(invoke<long double>(current_target, 22));
        auto maximum_speed = static_cast<long double>(invoke<long double>(entity, 22));
        if (maximum_speed < number(second_speed))
            maximum_speed = number(second_speed);
        attempt_tactic = number(stored_difference) < maximum_speed * .2f;
    }
    if (attempt_tactic) {
        const bool left = legacy::lateral_negative_for_target(entity, target);
        Dword now = graphics::unresolved::camera_state;
        if (at<Dword>(entity, 580) == 1 || at<Dword>(entity, 580) == 7 ||
            at<int>(entity, 592) <= int(now)) {
            const Dword choice = ai::random_word() % 12;
            at<Dword>(entity, 580) = choice < 4 ? 2 : choice < 7 ? 3 : choice < 9 ? 4 : 5;
            if (magnitude(number(target_speed)) < number(35.762401580810546875f)) {
                if (at<Dword>(entity, 580) == 6)
                    at<Dword>(entity, 580) = 3;
                else if (at<Dword>(entity, 580) == 5)
                    at<Dword>(entity, 580) = 2;
            }
            now = graphics::unresolved::camera_state;
            at<Dword>(entity, 592) = now + 900;
            at<Byte>(entity, 584) = 0;
            at<Dword>(entity, 596) = now + 300;
        }
        if (at<int>(entity, 596) <= int(now)) {
            const Byte previous = at<Byte>(entity, 584);
            at<Byte>(entity, 584) = previous == 0;
            at<Dword>(entity, 596) = now + (previous == 0 ? 240 : 30);
        }
        if (at<Byte>(entity, 584)) {
            longitudinal_offset = lateral_offset = 0;
            std::memcpy(target_point, invoke<const float *>(target, 0), 12);
            lateral_gain = 10;
            gap_gain = 2;
        } else {
            const float width =
                float(number(at<float>(at<void *>(at<void *>(entity, 368), 1768), 120)) +
                      at<float>(at<void *>(at<void *>(target, 368), 1768), 120));
            const float length =
                float(number(at<float>(at<void *>(at<void *>(entity, 368), 1768), 128)) +
                      at<float>(at<void *>(at<void *>(target, 368), 1768), 128));
            bool beside = true;
            switch (at<Dword>(entity, 580)) {
            default:
                longitudinal_offset = .5f;
                break;
            case 3:
                longitudinal_offset = float(number(length) + 1.f);
                beside = false;
                break;
            case 4:
                break;
            case 5:
                longitudinal_offset = -1;
                break;
            case 6:
                longitudinal_offset = float(-number(length) - .5f);
                beside = false;
                break;
            }
            if (beside)
                lateral_offset = float(number(width) + .5f);
            if (left)
                lateral_offset = -lateral_offset;
        }
        scale_vectors(1, pose + 9, lateral_offset, target_point);
        const float *position = invoke<const float *>(target, 0);
        for (unsigned axis = 0; axis < 3; ++axis)
            target_point[axis] = float(number(target_point[axis]) + position[axis]);
        use_target = true;
    }
    auto current_speed = magnitude(static_cast<long double>(invoke<long double>(entity, 22)));
    const float lookahead = number(8.9406f) < current_speed || float(current_speed) == 8.9406f
                                ? float(current_speed)
                                : 8.9406f;
    float destination[3], maximum_speed;
    Dword sampled_node;
    const float *position = invoke<const float *>(entity, 0);
    if (!entities::pending::sample_path_target(bytes(entity) + 220, destination, &maximum_speed,
                                               &sampled_node, pose, lookahead, position, 0))
        return 0;
    if (use_target) {
        float difference[3];
        position = invoke<const float *>(entity, 0);
        for (unsigned axis = 0; axis < 3; ++axis)
            difference[axis] = float(number(target_point[axis]) - position[axis]);
        const float projected = float(hardware::dot_product(difference, pose + 9) * lateral_gain);
        position = invoke<const float *>(entity, 0);
        const auto limit = drive_frame::point_distance(destination, position) * .15f;
        float offset = projected;
        if (!(number(projected) < limit) && float(limit) != projected)
            offset = float(limit);
        else if (number(projected) < -limit)
            offset = float(-limit);
        scale_vectors(1, pose + 9, offset, difference);
        for (unsigned axis = 0; axis < 3; ++axis)
            destination[axis] = float(number(destination[axis]) + difference[axis]);
    }
    float desired_speed = 0;
    if (!reverse_own_motion) {
        const float signed_gap = opposite_target ? -gap : gap;
        desired_speed = float(((number(signed_gap) + longitudinal_offset) - speed_bias) * gap_gain +
                              target_speed);
        if (opposite_target) {
            float minimum = 0;
            if (at<Dword>(chase, 88) == 1)
                minimum = float(magnitude(number(target_speed)) * .8f);
            if (desired_speed < minimum)
                desired_speed = minimum;
        }
    }
    float limit = 44.702999114990234375f;
    if (direction == 1) {
        const float path_limit = use_target ? float(number(target_speed) * 1.1f) : 44.702999114990234375f;
        if (maximum_speed > path_limit)
            limit = maximum_speed;
    }
    if (limit < desired_speed)
        desired_speed = limit;
    const float *target_position = invoke<const float *>(target, 0);
    position = invoke<const float *>(entity, 0);
    const float distance = float(drive_frame::point_distance(position, target_position));
    auto target_forward = magnitude(static_cast<long double>(invoke<long double>(target, 22)));
    if (target_forward < number(2.23515f) && distance < 5) {
        invoke(entity, 32, controls);
        return 0;
    }
    target_forward = magnitude(static_cast<long double>(invoke<long double>(target, 22)));
    if (target_forward < number(6.70545f) && gap < 20) {
        float heading[3], difference[3];
        invoke(entities::interface_at(entity, 3), 2, heading);
        const float *own_position = invoke<const float *>(entity, 0);
        target_position = invoke<const float *>(target, 0);
        for (unsigned axis = 0; axis < 3; ++axis)
            difference[axis] = float(number(target_position[axis]) - own_position[axis]);
        const bool behind = hardware::dot_product(heading, difference) < number(0);
        const float side = legacy::lateral_f32(entity);
        auto angle = calculate_route_heading(entity);
        if (behind) {
            angle += .5f;
            if (number(.5f) < angle)
                angle = angle - 1.f;
            desired_speed = -desired_speed;
        }
        const auto tenth = static_cast<long double>(.1);
        if ((side < -.75f && tenth < angle) || (side > .75f && angle < -tenth)) {
            at<float>(entity, 392) = behind ? 1.f : -1.f;
            desired_speed = behind ? 6.70545f : -6.70545f;
        }
        Dword speed_bits;
        std::memcpy(&speed_bits, &desired_speed, 4);
        invoke(entity, 33, speed_bits, controls);
        if (behind) {
            own_position = invoke<const float *>(entity, 0);
            for (unsigned axis = 0; axis < 3; ++axis)
                destination[axis] = float(number(own_position[axis]) - difference[axis]);
        } else
            std::memcpy(destination, invoke<const float *>(target, 0), 12);
        discard_steering(entity, destination, controls);
        return 0;
    }
    if (invoke<Byte>(entity, 45, controls, Dword(1087804172)))
        return 0;
    Dword speed_bits;
    std::memcpy(&speed_bits, &desired_speed, 4);
    invoke(entity, 54, destination, destination, speed_bits, &desired_speed, Dword(1086324736));
    std::memcpy(&speed_bits, &desired_speed, 4);
    invoke(entity, 33, speed_bits, controls);
    discard_steering(entity, destination, controls);
    return 1;
}

void update_cop_controls(void *entity, Dword controls_word) {
    Dword *controls = reinterpret_cast<Dword *>(controls_word);
    void *population = system(76);
    void *target = chased_entity(entity);
    const auto distance = static_cast<long double>(ai::pair_distance(entity, target));
    const float blend =
        !(distance <= number(10)) ? .15f : 0.039999999105930328369140625f;
    void *target_car = at<void *>(target, 368);
    at<float>(entity, 1848) =
        float((number(at<float>(target_car, 3136)) - at<float>(entity, 1848)) * blend +
              at<float>(entity, 1848));
    const bool enabled = lifecycle::state<Dword>(7352732) == 0;
    at<float>(entity, 1852) =
        float((number(at<float>(target_car, 3144)) - at<float>(entity, 1852)) * blend +
              at<float>(entity, 1852));
    if (!enabled) {
        invoke(entity, 32, controls);
        return;
    }
    if (relocate_distant_chase_cop(entity))
        return;
    if (invoke<Dword>(entity, 50)) {
        const int period = at<int>(population, 84540);
        if (period && int(graphics::unresolved::camera_state) % period < period / 2) {
            const auto steering =
                number(at<float>(at<void *>(entity, 368), 7992)) * 127.f;
            const float adjusted = float(
                steering + (!(steering >= 0.0L) ? 0.499998986721038818359375f : -0.499998986721038818359375f));
            const auto rounded = hardware::current_integer_f32(adjusted);
            at<Byte>(controls, 4) = Byte(Dword(rounded));
            // The controls word is pushed before slot22; it remains for slot33.
            const float speed = float(invoke<long double>(entity, 22));
            Dword bits;
            std::memcpy(&bits, &speed, 4);
            invoke(entity, 33, bits, controls);
            return;
        }
    }
    if (at<Dword>(entity, 580) == 7) {
        const auto speed = magnitude(static_cast<long double>(invoke<long double>(target, 22))) *
                               at<float>(population, 84528) +
                           at<float>(population, 84532);
        const float requested = number(at<float>(population, 84536)) < speed
                                    ? at<float>(population, 84536)
                                    : float(speed);
        pursuit_controls(entity, controls, requested);
    } else
        pursuit_controls(entity, controls, 0);
}
}
