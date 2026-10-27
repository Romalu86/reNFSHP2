#include "car_physics_frame.hpp"
#include "car_drive_frame_x87.hpp"
namespace nfs::game::runtime::physics::car_frame::drive {
void control_clamp_f32(float value, float minimum, float maximum, float *output) {
    if (value < minimum)
        value = minimum;
    else if (maximum < value)
        value = maximum;
    *output = value;
}

void transmission_clamp_f32(float value, float minimum, float maximum, float *output) {
    if (value < minimum)
        value = minimum;
    else if (maximum < value)
        value = maximum;
    *output = value;
}
} // namespace nfs::game::runtime::physics::car_frame::drive

namespace nfs::game::runtime::physics::car_frame {

void update_active_car_physics(void *car) {
    update_car_orientation_frame(car);
    float transformed[3];
    matrices::transform_vectors(1, vector_at(at<void *>(car, 1768), 20), vector_at(car, 412),
                                transformed);
    std::memcpy(vector_at(car, 3196), transformed, 12);
    matrices::transform_vectors(1, vector_at(at<void *>(car, 1768), 8), vector_at(car, 412),
                                transformed);
    std::memcpy(vector_at(car, 3160), transformed, 12);
    void *body = at<void *>(car, 1768);
    for (unsigned axis = 0; axis < 2; ++axis) {
        const unsigned angular_offset = axis ? 20 : 24;
        const unsigned radius_offset = axis ? 1808 : 1804;
        if (at<float>(body, angular_offset) != 0)
            at<float>(car, radius_offset) =
                float(drive::retained(at<float>(body, 16)) /
                      (drive::retained(at<float>(body, angular_offset)) * 6.283185482025146484375f));
        else
            at<Dword>(car, radius_offset) = 1249125376;
    }
    const float lateral = at<float>(car, 3160), forward = at<float>(car, 3168);
    const auto lateral_squared = drive::retained(lateral) * lateral;
    const auto forward_squared = drive::retained(forward) * forward;
    const float square = float(lateral_squared + forward_squared);
    float speed = float(__builtin_sqrtl(drive::retained(square)));
    if (forward < 0)
        speed = -speed;
    at<float>(car, 3132) = speed;
    at<float>(car, 476) =
        float(drive::direction(lateral, float(drive::absolute(drive::retained(forward)))));
    if ((at<Dword>(car, 476) | 2147483648u) == 4290772992u)
        at<Dword>(car, 476) = 0;
    sample_wheel_ground(car);
    update_wheel_surface_motion(car);
    update_car_throttle_and_brakes(car);
    for (unsigned offset : {2116u, 2120u, 2124u, 2412u, 2416u, 2420u, 3004u, 3008u, 3012u,
                            2708u, 2712u, 2716u})
        at<Dword>(car, offset) = 0;
    at<float>(car, 7992) = float(drive::steering_input(car));
    for (unsigned index = 0; index < 4; ++index)
        set_wheel_steering(wheel_at(car, index), at<float>(car, 7992));
    body = at<void *>(car, 1768);
    const auto clear_force = [&](unsigned offset) {
        at<Dword>(car, offset) = 0;
        at<Dword>(car, offset + 4) = 0;
        at<Dword>(car, offset + 8) = 0;
    };
    if (!at<Byte>(body, 730) && !(at<float>(body, 112) < .85f)) {
        const unsigned contacts = unsigned(at<Byte>(car, 2796)) + at<Byte>(car, 3092) +
                                  at<Byte>(car, 2500) + at<Byte>(car, 2204);
        if (contacts < 2) {
            for (unsigned offset : {2176u, 2472u, 3064u, 2768u})
                clear_force(offset);
            apply_aerodynamic_downforce(car);
            at<float>(car, 3152) = float(drive::retained(at<float>(car, 496)) *
                                          at<float>(at<void *>(car, 1768), 36) * -2.f);
            at<Dword>(car, 3156) = 0;
            at<Dword>(car, 3148) = 0;
            matrices::transform_vectors(1, vector_at(car, 3148), vector_at(car, 376),
                                        transformed);
            std::memcpy(vector_at(car, 3136), transformed, 12);
            integrate_car_linear_forces(car);
            apply_aerodynamic_drag(car);
            update_car_transmission(car);
            update_car_track_sample(car);
            return;
        }
        solve_wheel_suspension(car);
        for (unsigned index = 0; index < 4; ++index) {
            void *wheel = wheel_at(car, index);
            const Dword material = !at<Dword>(wheel, 72) ? 0 : at<Byte>(wheel, 126) & 31;
            at<Dword>(wheel, 132) = material;
            float grip = at<float>(wheel, 268);
            if (grip < .95f)
                grip = .95f;
            else if (grip > 1.05f)
                grip = 1.05f;
            grip = float(drive::retained(grip) * at<float>(wheel, 272));
            void *surface = pending::surface_properties(int(material));
            float static_grip = at<float>(surface, 4), sliding_grip = at<float>(surface, 8);
            if (car_is_racer(car)) {
                surface = pending::surface_properties(1);
                static_grip = at<float>(surface, 4);
                sliding_grip = at<float>(surface, 8);
            }
            at<float>(wheel, 136) = float(drive::retained(static_grip) * grip);
            at<float>(wheel, 140) = float(drive::retained(sliding_grip) * grip);
        }
        if (!race_started)
            for (unsigned offset : {3160u, 3164u, 3168u, 3148u, 3152u, 3156u, 3196u, 3200u,
                                    3204u, 3184u, 3188u, 3192u})
                at<Dword>(car, offset) = 0;
        solve_car_tyre_forces(car);
        if (drive::absolute(drive::retained(at<float>(car, 3148))) < drive::retained(.001f))
            at<Dword>(car, 3148) = 0;
        matrices::transform_vectors(1, vector_at(car, 3148), vector_at(car, 376), transformed);
        std::memcpy(vector_at(car, 3136), transformed, 12);
        integrate_car_linear_forces(car);
        if (at<int>(car, 448) > 2) {
            const auto stopped_metric =
                drive::absolute(drive::retained(at<float>(at<void *>(car, 1768), 128)) * at<float>(car, 3200)) +
                drive::absolute(drive::retained(at<float>(car, 3132)));
            const bool stopped = stopped_metric < drive::retained(.25f);
            if (drive::absolute(drive::retained(at<float>(car, 3132))) < drive::retained(6)) {
                float local[3];
                matrices::transform_vectors(1, vector_at(at<void *>(car, 1768), 20),
                                            vector_at(car, 412), local);
                if (drive::absolute(drive::retained(local[1])) < drive::retained(0.00277777784503996372222900390625f))
                    local[1] = 0;
                if (stopped)
                    local[1] = 0;
                body = at<void *>(car, 1768);
                matrices::transform_vectors(1, local, vector_at(car, 376), transformed);
                std::memcpy(vector_at(body, 20), transformed, 12);
            }
            if (at<int>(car, 448) > 2 && (stopped || at<float>(car, 564) > 0)) {
                float local[3];
                matrices::transform_vectors(1, vector_at(at<void *>(car, 1768), 8),
                                            vector_at(car, 412), local);
                if (at<float>(car, 564) > 0 && car_is_racer(car) &&
                    !invoke<Dword>(at<void *>(car, 7988), 50) &&
                    at<float>(at<void *>(car, 1768), 608) == 0 &&
                    drive::retained(at<float>(car, 7992)) * local[0] < drive::retained(0) &&
                    at<float>(car, 3168) > 6) {
                    const auto rear_slide_damping =
                        drive::absolute(drive::retained(at<float>(car, 564)) * 0.0333333350718021392822265625f * local[0]);
                    local[0] = float(!(local[0] >= 0) ? rear_slide_damping + local[0]
                                                    : drive::retained(local[0]) - rear_slide_damping);
                }
                if (stopped)
                    local[0] = 0;
                body = at<void *>(car, 1768);
                matrices::transform_vectors(1, local, vector_at(car, 376), transformed);
                std::memcpy(vector_at(body, 8), transformed, 12);
                update_car_track_sample(car);
                return;
            }
        }
    } else {
        for (unsigned offset : {2176u, 2472u, 3064u, 2768u})
            if (!at<Byte>(car, offset + 28) || at<float>(car, offset + 4) == 0)
                clear_force(offset);
        apply_airborne_resistance(car);
        update_car_transmission(car);
    }
    update_car_track_sample(car);
}

void update_kinematic_car_physics(void *car) {
    const float previous_speed = float(drive::absolute(drive::retained(at<float>(car, 3168))));
    void *body = at<void *>(car, 1768);
    float transformed[3];
    matrices::transform_vectors(1, vector_at(body, 20), vector_at(body, 72), transformed);
    std::memcpy(vector_at(car, 3196), transformed, 12);
    matrices::transform_vectors(1, vector_at(body, 8), vector_at(body, 72), transformed);
    std::memcpy(vector_at(car, 3160), transformed, 12);
    const float speed = float(drive::absolute(drive::retained(at<float>(car, 3168))));
    if (!(speed < .44703f))
        at<float>(car, 1608) = !(at<Byte>(car, 3476) & 2)
                                    ? 0
                                    : float(drive::retained(previous_speed) - (drive::retained(speed) - .44703f));
    else
        at<float>(car, 1608) = 1;
    sample_wheel_ground(car);
    float steering = 0;
    for (unsigned index = 0; index < 4; ++index)
        set_wheel_steering(wheel_at(car, index), 0);
    project_wheel_ground_speeds(car);
    for (unsigned index = 0; index < 4; ++index) {
        void *wheel = wheel_at(car, index);
        bool clamp = true;
        if (!(speed < 0.00044703003368340432643890380859375f)) {
            if (at<Byte>(wheel, 256)) {
                const auto angular_speed =
                    drive::retained(at<float>(wheel, 44)) / drive::retained(at<float>(wheel, 216));
                at<float>(wheel, 156) = float(angular_speed);
                at<float>(wheel, 264) =
                    float(angular_speed * 0.0333333350718021392822265625f + at<float>(wheel, 264));
                steering = float(drive::direction(
                    at<float>(car, 3196), float(drive::absolute(drive::retained(at<float>(car, 3204))))));
                Dword bits;
                std::memcpy(&bits, &steering, 4);
                if ((bits | 2147483648u) == 4290772992u) {
                    steering = 0;
                    clamp = false;
                }
            }
        } else {
            at<Dword>(wheel, 44) = 0;
            at<Dword>(wheel, 152) = 0;
            at<Dword>(wheel, 156) = 0;
        }
        if (clamp)
            steering = float(drive::control_clamp(steering, -.75f, .75f));
        set_wheel_steering(wheel, steering);
    }
    update_kinematic_engine_rpm(car);
}
} // namespace nfs::game::runtime::physics::car_frame
