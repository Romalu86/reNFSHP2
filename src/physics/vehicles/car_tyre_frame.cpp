#include "car_physics_frame.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include <intrin.h>
namespace nfs::game::runtime::physics::car_frame {
namespace {
long double hardware_number(float value) { return static_cast<long double>(value); }
long double hardware_magnitude(long double value) { return value < 0 ? -value : value; }
long double hardware_sine(float turns) { return turn_sine(turns); }
long double hardware_cosine(float turns) { return turn_cosine(turns); }
long double hardware_clamp_control(float value, float minimum, float maximum) {
    if (value < minimum)
        return hardware_number(minimum);
    if (maximum < value)
        return hardware_number(maximum);
    return hardware_number(value);
}
} // namespace

void solve_car_tyre_forces(void *car) {
    project_wheel_ground_speeds(car);
    float transformed[3];
    matrices::transform_vectors(1, vector_at(car, 3136), vector_at(car, 412), transformed);
    __movsd(reinterpret_cast<unsigned long *>(vector_at(car, 3148)),
            reinterpret_cast<const unsigned long *>(transformed), 3);
    at<Dword>(car, 3152) = 0;
    apply_lateral_weight_transfer(car);
    apply_body_roll_weight_transfer(car);
    apply_aerodynamic_drag(car);
    apply_aerodynamic_downforce(car);
    legacy::discard_rolling_resistance(car);
    update_car_transmission(car);
    solve_low_speed_wheel_forces(car);
    apply_car_brakes(car);
    if (race_started) {
        legacy::discard_clutch_limit();
        for (unsigned index = 0; index < 4; ++index) {
            void *wheel = wheel_at(car, index);
            integrate_wheel_angular_speed(wheel);
            const float compressed = float(hardware_number(at<float>(wheel, 232)) * .001f * .005f);
            auto deflection = hardware_clamp_control(compressed, 0, .05f);
            const float angular = at<float>(wheel, 156);
            const float stiffness = at<float>(car, index < 2 ? 944 : 1220);
            const float radius = at<float>(car, index < 2 ? 352 : 356);
            const auto angular_squared = hardware_number(angular) * angular;
            const auto radius_factor = hardware_number(radius) * inverse_gravity;
            const auto adjusted_force = (hardware_number(stiffness) * (radius_factor * angular_squared)) * .001f;
            const float adjusted = float(deflection - adjusted_force);
            deflection = hardware_clamp_control(adjusted, -.01f, stiffness);
            at<float>(wheel, 216) = float(hardware_number(radius) - deflection);
            at<Dword>(wheel, 148) = 0;
            at<float>(wheel, 264) =
                float(hardware_number(angular) * 0.005305165f + at<float>(wheel, 264));
            at<float>(wheel, 152) = float(hardware_number(angular) * at<float>(wheel, 216));
            at<float>(wheel, 144) =
                float(index < 2 ? hardware_number(at<float>(car, 112)) * at<float>(car, 3200) +
                                      at<float>(car, 3160)
                                : hardware_number(at<float>(car, 3160)) -
                                      hardware_number(at<float>(car, 116)) * at<float>(car, 3200));
            const float ground_speed = at<float>(wheel, 44);
            const auto difference =
                hardware_number(at<float>(wheel, 216)) * at<float>(wheel, 156) - ground_speed;
            float slip = 0;
            if (hardware_number(.25f) < hardware_magnitude(difference)) {
                auto retained_slip = difference;
                if (ground_speed != 0)
                    retained_slip = difference / hardware_number(ground_speed);
                slip = float(retained_slip);
            }
            at<float>(wheel, 32) = float(hardware_clamp_control(slip, -1, 100000));
            const auto front_camber = hardware_number(at<float>(car, 928)) * 0.0055555556900799274444580078125f;
            const float rear_camber = float(hardware_number(at<float>(car, 1204)) * 0.0055555556900799274444580078125f);
            auto camber = index < 2 ? front_camber : hardware_number(rear_camber);
            if (at<Byte>(wheel, 256))
                camber =
                    hardware_magnitude(hardware_number(at<float>(car, 368)) * (index < 2 ? 1.05f : 1.02f)) +
                    camber;
            auto *camber_slot = reinterpret_cast<volatile float *>(bytes(wheel) + 24);
            *camber_slot = float(camber);
            const float stored_camber = *camber_slot;
            *camber_slot = float(hardware_clamp_control(stored_camber, -0.083333336f, 0.083333336f));
        }
        for (unsigned index = 0; index < 4; ++index)
            limit_wheel_friction_forces(wheel_at(car, index));
    }
    for (unsigned index = 0; index < 4; ++index) {
        void *wheel = wheel_at(car, index);
        if (!at<Byte>(wheel, 256)) {
            at<Dword>(wheel, 228) = 0;
            at<Dword>(wheel, 232) = 0;
            at<Dword>(wheel, 236) = 0;
        }
    }
    for (unsigned index = 0; index < 4; ++index) {
        void *wheel = wheel_at(car, index);
        const float angle = at<float>(wheel, 16);
        if (angle != 0) {
            Dword lateral_bits, longitudinal_bits;
            std::memcpy(&lateral_bits, vector_at(wheel, 228), 4);
            std::memcpy(&longitudinal_bits, vector_at(wheel, 236), 4);
            const float cos_angle = float(hardware_cosine(angle));
            const auto sin_angle = hardware_sine(angle);
            float lateral, longitudinal;
            std::memcpy(&lateral, &lateral_bits, 4);
            std::memcpy(&longitudinal, &longitudinal_bits, 4);
            at<float>(wheel, 228) =
                float(hardware_number(cos_angle) * lateral + hardware_number(longitudinal) * sin_angle);
            at<float>(wheel, 236) = float(hardware_number(cos_angle) * longitudinal - sin_angle * lateral);
        }
    }
    accumulate_car_torques(car);
    const float inverse_mass = at<float>(at<void *>(car, 1768), 36);
    const auto weight = (hardware_number(at<float>(car, 2772)) + at<float>(car, 3068) +
                         at<float>(car, 2476) + at<float>(car, 2180)) *
                        inverse_gravity;
    if (!(hardware_number(0) < weight)) {
        at<Dword>(car, 3148) = 0;
        at<float>(car, 3152) = float(-(hardware_number(at<float>(car, 496)) * inverse_mass));
        at<float>(car, 3156) = float(hardware_number(at<float>(car, 480)) * inverse_mass);
    } else {
        at<float>(car, 3148) = float((hardware_number(at<float>(car, 2768)) + at<float>(car, 3064) +
                                       at<float>(car, 2472) + at<float>(car, 2176)) *
                                      inverse_mass);
        at<float>(car, 3152) = float(-(hardware_number(at<float>(car, 496)) * inverse_mass));
        at<float>(car, 3156) =
            float((hardware_number(at<float>(car, 480)) + at<float>(car, 2776) + at<float>(car, 3072) +
                   at<float>(car, 2480) + at<float>(car, 2184)) *
                  inverse_mass);
        if (race_started && hardware_magnitude(hardware_number(at<float>(car, 3168))) < hardware_number(.25f)) {
            float braking = at<float>(car, 1608);
            if (braking == 0) {
                if (at<Byte>(car, 1585))
                    braking = 1;
                else if (at<float>(car, 1612) == 0 && (wheel_reverse_brake(wheel_at(car, 0)) ||
                                                        wheel_reverse_brake(wheel_at(car, 1)) ||
                                                        wheel_reverse_brake(wheel_at(car, 2)) ||
                                                        wheel_reverse_brake(wheel_at(car, 3))))
                    braking = 1;
            }
            if (braking != 0) {
                const float remaining = float(hardware_number(1) - braking);
                at<float>(car, 3156) = float(hardware_number(braking) * at<float>(car, 3168) * -30.f);
                for (unsigned offset : {2104u, 2400u, 2992u, 2696u})
                    at<float>(car, offset) = float(hardware_number(at<float>(car, offset)) * remaining);
            }
        }
    }
    matrices::transform_vectors(1, vector_at(car, 3184), vector_at(car, 376), transformed);
    void *body = at<void *>(car, 1768);
    __movsd(reinterpret_cast<unsigned long *>(vector_at(car, 3172)),
            reinterpret_cast<const unsigned long *>(transformed), 3);
    matrices::transform_vectors(1, vector_at(car, 3196), vector_at(car, 376), transformed);
    __movsd(reinterpret_cast<unsigned long *>(vector_at(body, 20)),
            reinterpret_cast<const unsigned long *>(transformed), 3);
    if (race_started) {
        body = at<void *>(car, 1768);
        for (unsigned axis = 0; axis < 3; ++axis)
            at<float>(body, 20 + axis * 4) =
                float(hardware_number(at<float>(car, 3172 + axis * 4)) * 0.0333333350718021392822265625f +
                      at<float>(body, 20 + axis * 4));
    }
}
} // namespace nfs::game::runtime::physics::car_frame
