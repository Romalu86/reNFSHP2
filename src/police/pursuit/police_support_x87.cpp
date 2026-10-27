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
#include "../../physics/vehicles/car_physics_frame.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../ai/driving/ai_path_frame_x87.hpp"
#include "../../race/rules/race_rules_x87.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace nfs::game::world::camera { void state_orientation(void *, float *); }
namespace nfs::game::runtime::ai::cars {
void release_cop_reservation(void *, void *);
void update_cop_patrol_target(void *, Dword);
void *contact_entity(void *, int);
}
namespace nfs::game::runtime::ai::lifecycle::pending { extern float chase_transition_seconds; }
namespace nfs::game::runtime::ai::entities::pending {
void steer_helicopter_patrol(void *);
void steer_helicopter_pursuit(void *);
void fire_helicopter_projectiles(void *);
}

namespace nfs::game::runtime::ai::entities::pending::police_frame {
long double magnitude(long double value) {
    return value < static_cast<long double>(0.f) ? -value : value;
}
float limit_turn_speed(void *entity, const float *point, float speed) {
    const float angle =
        float(magnitude(police_math::steer_entity(entity, point, bytes(entity) + 416)));
    const auto difference =
        static_cast<long double>(1.f) - static_cast<long double>(turn_cosine(angle));
    if (static_cast<long double>(0.01f) < difference) {
        const auto quarter_turn_radians =
            static_cast<long double>(0.25f) * 6.2831854820251465f;
        const auto radius = static_cast<long double>(5.f) / difference;
        const float limit = float(quarter_turn_radians * radius);
        if (speed < -limit)
            speed = -limit;
        else if (limit < speed)
            speed = limit;
    }
    return speed;
}
void helicopter_path_target(void *entity, float *point) {
    float pose[21];
    void *path = bytes(entity) + 220;
    lifecycle::sample_path(path, 0, pose);
    if (at<int>(entity, 240) == -1)
        dependencies::reverse_section_pose(pose);
    const auto speed =
        magnitude(static_cast<long double>(invoke<long double>(entity, 22)));
    const float lookahead = speed < static_cast<long double>(8.9406f) ? 8.9406f : float(speed);
    float width;
    Dword sample;
    sample_path_target(path, point, &width, &sample, pose, lookahead,
                       invoke<const float *>(entity, 0), 0);
    point[1] = float(static_cast<long double>(point[1]) - 3.f);
}
}

namespace nfs::game::runtime::ai::entities::police_frame {
void __fastcall set_helicopter_speed_retained(void *entity, void *, float desired, Dword control_address) {

    auto X = [&](float v) { return static_cast<long double>(v); };
    at<float>(entity, 396) = desired;
    const auto current = static_cast<long double>(invoke<long double>(entity, 22));
    float &throttle = at<float>(entity, 412);
    if (current < X(desired)) {
        const auto increased = X(0.06f) + throttle;
        throttle = float(increased);
        if (X(1.f) < increased)
            throttle = 1.f;
    } else if (X(desired) < current) {
        const auto decreased = X(throttle) - 0.06f;
        throttle = float(decreased);
        if (decreased < X(-1.f))
            throttle = -1.f;
    } else if (throttle < 0.f) {
        const auto increased = X(0.06f) + throttle;
        throttle = float(increased);
        if (increased > X(0.f))
            throttle = 0;
    } else if (throttle > 0.f) {
        const auto decreased = X(throttle) - 0.06f;
        throttle = float(decreased);
        if (decreased < X(0.f))
            throttle = 0;
    }
    auto difference = current - X(desired);
    if (difference < X(0.f))
        difference = -difference;
    if (difference < X(4.4703f)) {
        const float bound = float(difference * 0.22369861602783203125f);
        float limited = -bound;
        if (!(throttle < limited)) {
            limited = throttle;
            if (bound < throttle)
                limited = bound;
        }
        const auto limited_part = X(limited) * 0.8f;
        const auto previous_part = X(throttle) * 0.2f;
        throttle = float(limited_part + previous_part);
    }
    const bool braking = !(throttle >= 0.f);
    const auto amount = X(throttle) * (braking ? -255.f : 255.f);
    int value = int(Dword(police_math::truncate(amount)));
    if (value < 0)
        value = 0;
    else if (value > 255)
        value = 255;
    void *controls = reinterpret_cast<void *>(control_address);
    at<Byte>(controls, braking ? 5 : 6) = 0;
    at<Byte>(controls, braking ? 6 : 5) = Byte(value);
}
}

namespace nfs::game::runtime::ai::entities::pending::police_frame {
void calculate_projectile_velocity_retained(void *entity, float *velocity) {

    float pose[21];
    lifecycle::sample_path(bytes(entity) + 220, 0, pose);
    auto spread = static_cast<long double>(0.f);
    const int pattern = at<int>(entity, 336), shot = at<int>(entity, 344);
    if (pattern == 0)
        spread = static_cast<long double>(shot == 2 ? 0.f : shot == 0 || shot == 3 ? -1.f : 1.f);
    else if (pattern == 1) {
        const auto index = static_cast<long double>(shot);
        spread = (index + index - 5.f + 1.f) * 0.25f;
    } else {
        spread = static_cast<long double>(lifecycle::random_fraction());
        spread = spread + spread - 1.f;
    }
    spread = spread * static_cast<long double>(at<int>(entity, 340));
    const float flight_time = at<float>(entity, 1672);
    if (flight_time <= 0.1f) {
        std::memcpy(velocity, runtime::saved::zero_vector, 12);
        return;
    }
    const float offset = float((static_cast<long double>(0.7f) / static_cast<long double>(flight_time)) * spread);
    const bool positive = !(offset < 0);
    scale_vectors(1, pose + (positive ? 15 : 12), positive ? offset : -offset, velocity);
    velocity[1] = -at<float>(entity, 1676);
    velocity[2] = -velocity[2];
}
}

namespace nfs::game::runtime::ai::entities::pending::police_frame {
void launch_projectile_retained(void *projectile, void *helicopter, const float *velocity) {

    at<Dword>(projectile, 240) = graphics::unresolved::camera_state;
    at<void *>(projectile, 244) = helicopter;
    void *source_body = invoke<void *>(interface_at(helicopter, 1, 320), 1);
    at<Dword>(source_body, 676) = 0;
    Dword position[3];
    std::memcpy(position, bytes(source_body) + 680, 12);
    using ActivateMethod = void(__thiscall *)(void *, Dword);
    const auto *activation_methods = *reinterpret_cast<const ActivateMethod **>(projectile);
    at<Byte>(projectile, 260) = 1;
    activation_methods[6](projectile, Dword{1});
    void *shape = at<void *>(projectile, 236);
    void *body = at<void *>(shape, 80);
    at<volatile Byte>(shape, 97) = 0;
    at<volatile Byte>(shape, 98) = 1;
    if (!body) {
        lifecycle::pending::bind_shape_body(shape, 1);
        body = at<void *>(shape, 80);
        at<Dword>(body, 676) = 0;
    }
    std::memcpy(bytes(body) + 32, &lifecycle::pending::projectile_mass, sizeof(Dword));
    at<Dword>(body, 464) = 0;
    at<Dword>(body, 60) = 0;
    at<void *>(projectile, 224) = body;
    const Dword x = ai::random_word();
    const float angular_x = float(static_cast<long double>(x % 100) * 0.01f);
    const Dword y = ai::random_word();
    const float angular_y = float(static_cast<long double>(y % 100) * 0.01f);
    const Dword z = ai::random_word();
    const auto angular_z = static_cast<long double>(z % 100);
    at<float>(body, 20) = angular_x;
    at<float>(body, 24) = angular_y;
    const Dword elasticity = *reinterpret_cast<const volatile Dword *>(
        &lifecycle::pending::projectile_elasticity);
    const auto scaled_angular_z = angular_z * 0.01f;
    at<Dword>(body, 56) = elasticity;
    at<float>(body, 28) = float(scaled_angular_z);
    std::memcpy(bytes(body) + 8, velocity, 12);
    std::memcpy(bytes(body) + 680, position, 12);
    std::memcpy(bytes(body) + 692, bytes(source_body) + 692, 36);
    matrices::transpose(reinterpret_cast<const float *>(bytes(body) + 692),
                        reinterpret_cast<float *>(bytes(body) + 72));
    at<Dword>(projectile, 248) = 0;
    at<Dword>(projectile, 252) = 0;
    at<Dword>(projectile, 256) = 5;
    if (void *emitter = invoke<void *>(interface_at(at<void *>(projectile, 244), 1, 320), 1))
        play_projectile_impact(at<float>(emitter, 680), at<float>(emitter, 684),
                               at<float>(emitter, 688), 50, 100);
}
}

namespace nfs::game::runtime::ai::entities::pending::police_frame {
void fire_helicopter_projectiles_retained(void *entity) {

    Dword *pool = ai::collision_state();
    if (!at<Dword>(entity, 344)) {
        const int count = int(pool[8]);
        int available = 0;
        for (int index = 0; index < count; ++index)
            if (void *projectile = reinterpret_cast<void *>(pool[3 + index]))
                if (!at<Byte>(projectile, 232))
                    ++available;
        if (available < (count < 5 ? count : 5))
            return;
    }
    const Dword now = graphics::unresolved::camera_state;
    if (at<int>(entity, 1664) > int(now))
        return;
    if (at<std::int16_t>(invoke<void *>(interface_at(entity, 1, 320), 1), 584) == 4)
        return;
    void *target = at<void *>(at<void *>(entity, 408), 44);
    const float *source_position = invoke<const float *>(entity, 0);
    const float *target_position = invoke<const float *>(target, 0);
    float delta[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        delta[axis] = float(static_cast<long double>(source_position[axis]) - target_position[axis]);
    if (delta[1] < 0 || lifecycle::pending::projectile_drop_height < delta[1])
        return;
    float heading[] = {at<float>(invoke<void *>(interface_at(target, 3), 1), 8), 0,
                       -at<float>(invoke<void *>(interface_at(target, 3), 1), 16)};
    if (heading[0] != 0 || heading[2] != 0)
        normalize_vector(heading, heading);
    else
        heading[2] = -1;
    if (!at<Dword>(entity, 1668))
        return;
    float velocity[3];
    calculate_projectile_velocity(entity, velocity);
    for (int index = 0; index < int(pool[8]); ++index) {
        void *projectile = reinterpret_cast<void *>(pool[3 + index]);
        if (!projectile || at<Byte>(projectile, 232))
            continue;
        at<Byte>(projectile, 232) = 1;
        launch_projectile(projectile, entity, velocity);
        at<Dword>(entity, 344) += 1;
        if (at<int>(entity, 344) >= 5 || at<int>(entity, 344) >= int(pool[8])) {
            ++at<Dword>(entity, 352);
            at<Dword>(entity, 344) = 0;
            at<Dword>(entity, 336) = ai::random_word() % 3;
            at<Dword>(entity, 340) = ai::random_word() * 2 - 1;
        }
        const unsigned minimum_address = at<Dword>(entity, 344) ? 7352832 : 7352824;
        const unsigned maximum_address = minimum_address - 4;
        const Dword minimum = lifecycle::state<Dword>(minimum_address);
        const Dword range = lifecycle::state<Dword>(maximum_address) - minimum;
        at<Dword>(entity, 1664) = minimum + now;
        if (range)
            at<Dword>(entity, 1664) += ai::random_word() % (range + 1);
        return;
    }
}
}

namespace nfs::game::runtime::ai::entities::pending::police_frame {
void steer_helicopter_patrol_retained(void *entity) {

    auto *population = static_cast<Dword *>(system(76));
    const float *position = invoke<const float *>(entity, 0);
    if (!lifecycle::player_within_range(population, position, 300.f)) {
        at<Dword>(entity, 380) = 0;
        invoke(entity, 6, Dword{0});
        ai::collision_state();
        at<Dword>(entity, 436) = 0;
        at<Byte>(entity, 348) = 0;
        return;
    }
    invoke<void *>(interface_at(entity, 1, 320), 1);
    int direction;
    float distance;
    Dword branch;
    if (!target_route_branch(entity, &direction, &distance, &branch))
        return;
    void *section = invoke<void *>(entity, 1);
    const int sample = invoke<int>(entity, 2);
    float tangent[3];
    dependencies::sample_horizontal_tangent(section, sample, tangent);
    void *body = invoke<void *>(interface_at(entity, 1, 320), 1);
    const float x = float(static_cast<long double>(tangent[0]) * at<float>(body, 8));
    body = invoke<void *>(interface_at(entity, 1, 320), 1);
    auto forward = static_cast<long double>(x) -
                   static_cast<long double>(tangent[2]) * at<float>(body, 16);
    int moving_direction = 1;
    if (forward < static_cast<long double>(0.f)) {
        forward = -forward;
        moving_direction = -1;
    }
    bool reversing = false;
    if (direction != moving_direction && static_cast<long double>(13.4109f) < forward) {
        direction = moving_direction;
        reversing = true;
    }
    at<int>(entity, 332) = direction;
    if (direction != at<int>(entity, 240))
        reverse_entity_path(entity);
    float point[3];
    helicopter_path_target(entity, point);
    float speed = 0;
    if (!reversing) {
        speed = distance;
        const float limit = at<float>(entity, 400);
        if (speed < 0.f)
            speed = 0.f;
        else if (speed > limit)
            speed = limit;
    }
    speed = limit_turn_speed(entity, point, speed);
    invoke(entity, 33, speed, reinterpret_cast<Dword>(bytes(entity) + 416));
}
}

namespace nfs::game::runtime::ai::entities::pending::police_frame {
void steer_helicopter_pursuit_retained(void *entity) {

    void *chase = at<void *>(entity, 408);
    void *target = at<void *>(chase, 44);
    at<Dword>(entity, 1668) = 0;
    at<float>(entity, 1672) = -1;
    at<Dword>(entity, 1676) = 0;
    ai::pair_distance(entity, target);
    const int deadline = at<int>(entity, 1652);
    if (deadline > 0 && deadline < int(graphics::unresolved::camera_state) &&
        at<int>(system(76), 84628) <= at<int>(entity, 352)) {
        if (at<Dword>(entity, 408) && at<Byte>(entity, 349))
            release_chase_helicopter(at<void *>(entity, 408));
        return;
    }
    invoke<void *>(interface_at(entity, 1, 320), 1);
    int direction;
    float distance;
    Dword branch;
    if (!target_route_branch(entity, &direction, &distance, &branch))
        return;
    target = at<Dword>(entity, 408) ? at<void *>(at<void *>(entity, 408), 44) : nullptr;
    void *section = invoke<void *>(entity, 1);
    const int sample = invoke<int>(entity, 2);
    auto X = [&](float value) { return static_cast<long double>(value); };
    float tangent[3];
    dependencies::sample_horizontal_tangent(at<void *>(entity, 440), at<int>(entity, 444),
                                            tangent);
    void *car = at<void *>(target, 368);
    void *car_body = at<void *>(car, 1768);
    const auto target_projection =
        (-X(at<float>(car_body, 16)) - X(at<float>(car, 3144)) * 3.f) * tangent[2] +
        (X(at<float>(car, 3136)) * 3.f + at<float>(car_body, 8)) * tangent[0];
    float target_speed = float(target_projection);
    const int target_direction = target_projection < X(0.f) ? -1 : 1;
    if (target_direction == -1)
        target_speed = -target_speed;
    dependencies::sample_horizontal_tangent(section, sample, tangent);
    void *body = invoke<void *>(interface_at(entity, 1, 320), 1);
    const float x = float(X(tangent[0]) * at<float>(body, 8));
    body = invoke<void *>(interface_at(entity, 1, 320), 1);
    auto forward = X(x) - X(tangent[2]) * at<float>(body, 16);
    int moving_direction = 1;
    if (forward < X(0.f)) {
        forward = -forward;
        moving_direction = -1;
    }
    int target_orientation = direction;
    if (section != at<void *>(entity, 440)) {
        const auto count = at<std::uint16_t>(entity, 1648);
        if (count)
            section = at<void *>(entity, 448 + (count - 1) * 4);
        const Dword endpoint = at<Dword>(at<void *>(entity, 440), 24);
        target_orientation =
            endpoint == at<Dword>(section, 20) || endpoint == at<Dword>(section, 24) ? -1 : 1;
    }
    bool intercept = false;
    if (target_orientation != target_direction) {
        const auto closing_window = (X(target_speed) + forward) * 4.f;
        if (closing_window >= X(distance)) {
            direction = int(0u - Dword(direction));
            intercept = true;
        }
    }
    bool reversing = false;
    if (direction != moving_direction && X(13.4109f) < forward) {
        reversing = true;
        direction = moving_direction;
    }
    at<int>(entity, 332) = direction;
    if (direction != at<int>(entity, 240))
        reverse_entity_path(entity);
    float point[3];
    helicopter_path_target(entity, point);
    body = invoke<void *>(interface_at(entity, 1, 320), 1);
    void *target_body = invoke<void *>(interface_at(target, 3), 0);
    const auto height = magnitude(X(at<float>(body, 684)) - at<float>(target_body, 684));
    volatile float &gravity = physics::car_frame::gravity;
    const auto terminal_velocity = height - X(gravity) * 0.5f;
    if (terminal_velocity <= X(0.f)) {
        const float squared_time = float((X(2.f) / gravity) * height);
        const volatile float flight_time = std::sqrt(squared_time);
        at<Dword>(entity, 1676) = 0;
        at<float>(entity, 1672) = flight_time;
    } else {
        at<float>(entity, 1676) = float(terminal_velocity);
        if (!(terminal_velocity <= X(8.9406f))) {
            const auto product = height * gravity;
            at<float>(entity, 1676) = 8.9406f;
            const float squared_speed = float(product + product + 79.93433380126953f);
            const volatile float root = std::sqrt(squared_speed);
            at<float>(entity, 1672) =
                float((X(root) - at<volatile float>(entity, 1676)) / gravity);
        } else
            at<float>(entity, 1672) = 1;
    }
    const auto lead = (X(at<float>(entity, 1672)) + 1.f) * target_speed + 5.f;
    at<Dword>(entity, 1668) = intercept && lead - X(target_speed) * 0.5f < X(distance) &&
                              X(distance) < X(target_speed) + lead;
    float speed = 0;
    if (!reversing) {
        speed = float((intercept ? -X(distance) : X(distance)) + lead + target_speed);
        if (speed < 0.f)
            speed = 0;
        else if (at<float>(entity, 400) < speed)
            speed = at<float>(entity, 400);
    }
    const float *target_position = invoke<const float *>(target, 0);
    const float *position = invoke<const float *>(entity, 0);
    const auto separation_x = X(position[0]) - target_position[0];
    const auto separation_y = X(position[1]) - target_position[1];
    const auto separation_z = X(position[2]) - target_position[2];
    volatile float separation[3];
    separation[0] = float(separation_x);
    separation[1] = float(separation_y);
    separation[2] = float(separation_z);
    const float squared_distance =
        float(X(separation[2]) * separation[2] + X(separation[0]) * separation[0]);
    const float horizontal_distance = std::sqrt(squared_distance);
    const auto target_forward =
        magnitude(static_cast<long double>(invoke<long double>(target, 22)));
    if (target_forward < X(2.23515f) && horizontal_distance < 5.f) {
        at<Byte>(entity, 420) = 0;
        invoke(entity, 33, 0.f, reinterpret_cast<Dword>(bytes(entity) + 416));
        return;
    }
    speed = limit_turn_speed(entity, point, speed);
    invoke(entity, 33, speed, reinterpret_cast<Dword>(bytes(entity) + 416));
}
}

namespace nfs::game::runtime::ai::entities::pending::police_frame {
void apply_helicopter_controls_retained(void *entity, float desired_speed, const float *target) {

    void *body = invoke<void *>(interface_at(entity, 1, 320), 1);
    const Byte accelerator = invoke<Byte>(entity, 17), brake = invoke<Byte>(entity, 18);
    auto X = [&](float value) { return static_cast<long double>(value); };
    float steering = float(static_cast<long double>(invoke<long double>(entity, 19)) *
                           0.0078740157186985015869140625f);
    const auto speed = static_cast<long double>(invoke<long double>(entity, 22));
    if (speed < X(0.f))
        steering = -steering;
    const float limit = at<float>(entity, 400);
    const Dword pedal_difference = Dword(accelerator) - Dword(brake);
    const float next_speed =
        float((X(1.f) - X(0.894060075283050537109375f) / X(limit)) * speed +
              static_cast<long double>(std::int32_t(pedal_difference)) * 0.0035061179660260677337646484375f);
    float bounded = next_speed;
    if (next_speed < -limit)
        bounded = -limit;
    else if (limit < next_speed)
        bounded = limit;
    float target_height = target[1];
    if (at<float>(body, 484) > 0) {
        const float clearance = float(X(at<float>(body, 684)) - at<float>(body, 484) + 5.5f);
        if (target_height < clearance)
            target_height = clearance;
    }
    invoke(entity, 24, 0u, float((X(target_height) - at<float>(body, 684)) * 2.f), bounded);
    const auto pitch_value = (X(desired_speed) - bounded) * 0.0024855402298271656036376953125f;
    float pitch = float(pitch_value);
    if (pitch_value < X(-0.0416666679084300994873046875f))
        pitch = -0.0416666679084300994873046875f;
    else if (pitch > 0.0416666679084300994873046875f)
        pitch = 0.0416666679084300994873046875f;
    auto roll = X(bounded) * steering * -0.0024855402298271656036376953125f;
    if (roll < X(-0.0555555559694766998291015625f))
        roll = X(-0.0555555559694766998291015625f);
    else if (X(0.0555555559694766998291015625f) < roll)
        roll = X(0.0555555559694766998291015625f);
    at<float>(body, 20) = float(X(pitch) - at<float>(entity, 384));
    at<float>(body, 24) = float(X(steering) * 0.25f);
    at<float>(body, 28) = float(roll - at<float>(entity, 392));
}
}

namespace nfs::game::runtime::ai::entities::pending::police_frame {
void apply_projectile_pursuit_damage_retained(void *cop, float damage) {

    if (damage < 0)
        damage = 0;
    else if (damage > 1.f)
        damage = 1.f;
    void *chase = at<void *>(cop, at<const int *>(cop, 4)[4] + 12);
    if (!chase)
        return;
    const auto increased = static_cast<long double>(damage) + at<float>(cop, 660);
    const auto limited = increased < static_cast<long double>(1.f) ? increased : static_cast<long double>(1.f);
    at<float>(cop, 660) = float(increased);
    at<float>(cop, 660) = float(limited);
    if (!(limited < static_cast<long double>(1.f)))
        set_police_chase_outcome(chase, 1);
}
}

namespace nfs::game::runtime::ai::entities::pending::police_frame {
void play_projectile_impact_retained(float x, float y, float z, Dword event, int maximum_distance) {

    const int bank = game::audio::state::banks()[2].handle;
    const float position[] = {x, y, z};
    for (int listener = 0; listener < race_audio::pending::race_audio_listener_count; ++listener) {
        void *state = invoke<void *>(game::tick_services[listener], 2);
        float camera[3], orientation[9];
        std::memcpy(camera, bytes(state) + 4, 12);
        game::world::camera::state_orientation(state, orientation);
        const float distance = float(police_math::point_distance(camera, position));
        int rounded = hardware::current_integer_f32(distance);
        if (rounded < 0 || rounded > 10000)
            rounded = 10000;
        if (maximum_distance < rounded)
            return;
        const int gain = race_audio::group_volume(127, 6, 127);
        int volume = int(
            Dword(police_math::truncate((static_cast<long double>(gain) * 1.25f))));
        if (volume < 0)
            volume = 0;
        else if (volume > 127)
            volume = 127;
        const Dword pan = race_audio::pending::race_audio_listener_count == 2
                             ? (listener ? 16383 : 49152)
                             : race_audio::pending::spatial_pan(camera, position);
        game::audio::play_bank_event(bank, int(event), std::uint16_t(pan), Byte(volume), 64);
    }
}
}

namespace nfs::game::runtime::ai::entities::police_frame {
Byte settle_spike_body_retained(void *entity) {

    void *body = invoke<void *>(interface_at(entity, 3), 1);
    if (!body)
        return 0;
    float position[3];
    std::memcpy(position, bytes(body) + 680, 12);
    alignas(4) Byte ground[84];
    physics::clear_ground_contact(ground);
    if (!physics::surface_height(ground, position))
        return 0;
    float cross[3], orientation[9];
    const float *normal = reinterpret_cast<const float *>(ground + 12);
    runtime::cross_vectors_entry(normal, reinterpret_cast<const float *>(bytes(body) + 692),
                                 cross);
    runtime::cross_vectors_entry(cross, normal, orientation);
    if (orientation[0] == 0 && orientation[1] == 0 && orientation[2] == 0) {
        at<Byte>(entity, 233) = 0;
        return 0;
    }
    normalize_vector(orientation, orientation);
    std::memcpy(orientation + 3, normal, 12);
    runtime::cross_vectors_entry(orientation, orientation + 3, orientation + 6);
    for (unsigned axis = 0; axis < 3; ++axis)
        position[axis] =
            float(static_cast<long double>(normal[axis]) * at<float>(body, 124) +
                  position[axis]);
    std::memcpy(bytes(body) + 680, position, 12);
    std::memcpy(bytes(body) + 692, orientation, 36);
    matrices::transpose(reinterpret_cast<float *>(bytes(body) + 692),
                        reinterpret_cast<float *>(bytes(body) + 72));
    update_spike_corners(entity);
    void *shape = at<void *>(entity, 228);
    dependencies::synchronize_track_vehicle(shape);
    physics::mark_track_vehicle_moved(shape);
    at<Byte>(entity, 233) = 0;
    return 1;
}
}

namespace nfs::game::runtime::ai::cars::police_frame {
Dword spike_wheel_hit_mask_retained(void *spike, void *entity) {

    if (!at<Byte>(spike, 212) || !at<Byte>(spike, 232) || !at<Dword>(spike, 284) ||
        !at<Byte>(entity, 212))
        return 0;
    void *car = at<void *>(entity, 368);
    if (!car)
        return 0;
    void *body = at<void *>(car, 1768);
    if (at<std::uint16_t>(body, 596))
        return 0;
    void *spike_body = invoke<void *>(entities::interface_at(spike, 3), 1);
    void *body_view = entities::interface_at(spike, 3);
    const float distance =
        float(police_math::point_distance(reinterpret_cast<const float *>(bytes(spike_body) + 680),
                                      reinterpret_cast<const float *>(bytes(body) + 680)));
    spike_body = invoke<void *>(body_view, 1);
    auto E = [](float value) { return static_cast<long double>(value); };
    if (E(at<float>(spike_body, 132)) * 3.f < E(distance))
        return 0;
    entities::update_spike_corners(spike);
    const float *rotation = reinterpret_cast<const float *>(bytes(body) + 72);
    float local_wheels[12], world_wheels[12];
    for (unsigned wheel = 0; wheel < 4; ++wheel)
        std::memcpy(local_wheels + wheel * 3, bytes(car) + 2224 + wheel * 296, 12);
    float position[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        at<Dword>(position, axis * 4) = at<Dword>(at<void *>(car, 1768), 680 + axis * 4);
    matrices::transform_positions(4, local_wheels,
                                  reinterpret_cast<const float *>(bytes(car) + 376), position,
                                  world_wheels, 12);
    auto velocity = E(at<float>(car, 3160)) * 0.05f;
    const float right = float((velocity < E(0) ? E(0) : velocity) + 0.25f);
    const float left = float((E(0) < velocity ? E(0) : velocity) - 0.25f);
    velocity = E(at<float>(car, 3168)) * 0.05f;
    const float front = float((velocity < E(0) ? E(0) : velocity) + 0.5f);
    const float rear = float((E(0) < velocity ? E(0) : velocity) - 0.5f);
    Dword hit_mask = 0;
    for (unsigned wheel = 0; wheel < 4; ++wheel) {
        if (!at<Byte>(car, 2204 + wheel * 296))
            continue;
        Dword outside[4];
        for (unsigned corner = 0; corner < 4; ++corner) {
            float delta[3], relative[3];
            delta[0] = float(E(at<float>(spike, 236 + corner * 12)) - world_wheels[wheel * 3]);
            outside[corner] = 0;
            delta[1] = float(E(at<float>(spike, 240 + corner * 12)) - world_wheels[wheel * 3 + 1]);
            delta[2] = float(E(at<float>(spike, 244 + corner * 12)) - world_wheels[wheel * 3 + 2]);
            matrices::transform_vectors(1, delta, rotation, relative);
            if (relative[0] > right)
                outside[corner] = 32;
            if (relative[2] > front)
                outside[corner] |= 2;
            if (relative[0] < left)
                outside[corner] |= 16;
            if (relative[2] < rear)
                outside[corner] |= 1;
        }
        if (!(outside[3] & outside[2] & outside[1] & outside[0]))
            hit_mask |= 1u << wheel;
    }
    return hit_mask;
}
}
