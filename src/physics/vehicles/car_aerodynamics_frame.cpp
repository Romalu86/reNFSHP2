#include "car_physics_frame.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <intrin.h>
namespace nfs::game::runtime::physics::car_frame {
namespace {
long double N(float value) { return static_cast<long double>(value); }
long double A(long double value) { return value < 0.0L ? -value : value; }
}
namespace {
void add_normal_load(void *car, unsigned offset, long double amount) {
    const auto force = N(at<float>(car, offset)) + amount;
    volatile float &slot = at<float>(car, offset);
    slot = float(force);
    slot = N(0) < force ? float(force) : 0;
}
} // namespace

void apply_body_air_resistance(void *car) {
    void *body = at<void *>(car, 1768);
    if (!at<std::int16_t>(body, 596) || 0.f < at<float>(car, 3164))
        return;
    float acceleration[3] = {0,
                             float(N(at<float>(body, 128)) * at<float>(body, 120) * 4.f *
                                   at<float>(car, 516) * at<float>(car, 3164) *
                                   at<float>(car, 3164) * .601f * at<float>(body, 36)),
                             0};
    float world_acceleration[3] = {};
    matrices::transform_vectors(1, acceleration, vector_at(body, 692), world_acceleration);
    for (float &component : world_acceleration)
        component = float(N(component) * 0.0333333350718021392822265625f);
    for (unsigned axis = 0; axis < 3; ++axis)
        at<float>(body, 8 + axis * 4) =
            float(N(world_acceleration[axis]) + at<float>(body, 8 + axis * 4));
}

void apply_aerodynamic_downforce(void *car) {
    at<Dword>(car, 496) = 0;
    const float speed = float(A(N(at<float>(car, 3168))));
    const auto pressure = N(speed) * gravity * .5f;
    at<float>(car, 488) = float(N(at<float>(car, 504)) * pressure);
    at<float>(car, 492) = float(pressure * at<float>(car, 508));
    add_normal_load(car, 2180, N(at<float>(car, 488)));
    add_normal_load(car, 2476, N(at<float>(car, 488)));
    add_normal_load(car, 3068, N(at<float>(car, 492)));
    add_normal_load(car, 2772, N(at<float>(car, 492)));
    at<float>(car, 496) = float((N(at<float>(car, 492)) + at<float>(car, 492)) +
                                  (N(at<float>(car, 488)) + at<float>(car, 488)));
}

void apply_aerodynamic_drag(void *car) {
    apply_body_air_resistance(car);
    const float speed = float(A(N(at<float>(car, 3168))));
    const float area = at<float>(car, 120);
    const float drag = float(N(speed) * speed * area * at<float>(car, 500) * .601f);
    at<float>(car, 480) = drag;
    if (at<float>(car, 3168) >= 0)
        at<float>(car, 480) = -drag;
    const auto moment = (N(at<float>(car, 544)) * .9090909f +
                         turn_sine(at<float>(car, 364)) * at<float>(car, 528)) *
                        drag;
    at<float>(car, 484) = float(moment);
    if (at<std::int16_t>(at<void *>(car, 1768), 596))
        return;
    const auto front = (moment / N(at<float>(car, 112))) * .5f;
    add_normal_load(car, 2180, -front);
    add_normal_load(car, 2476, -front);
    const auto rear = (moment / N(at<float>(car, 116))) * .5f;
    add_normal_load(car, 3068, rear);
    add_normal_load(car, 2772, rear);
    if (at<Byte>(car, 2204) && at<Byte>(car, 2500) && at<Byte>(car, 3092) &&
        at<Byte>(car, 2796)) {
        const auto lift = N(at<float>(car, 512)) * speed * speed * area * -.601f * .25f;
        for (unsigned offset : {2180u, 2476u, 3068u, 2772u})
            add_normal_load(car, offset, lift);
    }
}

void apply_airborne_resistance(void *car) {
    void *body = at<void *>(car, 1768);
    if (at<float>(body, 112) > .75f && at<float>(body, 592) > 10) {
        apply_aerodynamic_downforce(car);
        body = at<void *>(car, 1768);
        float local[3] = {0, float(-(N(at<float>(car, 496)) * at<float>(body, 36))), 0};
        float world[3];
        matrices::transform_vectors(1, local, vector_at(body, 692), world);
        for (unsigned axis = 0; axis < 3; ++axis)
            at<float>(body, 8 + axis * 4) =
                float(N(world[axis]) * 0.0333333350718021392822265625f + at<float>(body, 8 + axis * 4));
    }
    if (at<std::int16_t>(body, 596)) {
        const auto x_drag = (A(N(at<float>(body, 716))) * .002f +
                             A(N(at<float>(body, 704))) * .006f +
                             A(N(at<float>(body, 692))) * .004f) *
                            (N(at<float>(body, 8)) * at<float>(body, 8)) * 0.0333333350718021392822265625f;
        const float z_drag = float(N(at<float>(body, 16)) * at<float>(body, 16) *
                                   (A(N(at<float>(body, 724))) * .002f +
                                    A(N(at<float>(body, 712))) * .006f +
                                    A(N(at<float>(body, 700))) * .004f) *
                                   0.0333333350718021392822265625f);
        at<float>(body, 8) =
            float(N(at<float>(body, 8)) + (at<float>(body, 8) > 0 ? -x_drag : x_drag));
        at<float>(body, 16) =
            float(N(at<float>(body, 16)) + (at<float>(body, 16) > 0 ? -z_drag : z_drag));
    }
}

void integrate_car_linear_forces(void *car) {
    void *body = at<void *>(car, 1768);
    if (race_started) {
        for (unsigned axis = 0; axis < 3; ++axis)
            at<float>(body, 8 + axis * 4) =
                float(N(at<float>(car, 3136 + axis * 4)) * 0.0333333350718021392822265625f +
                      at<float>(body, 8 + axis * 4));
        return;
    }
    float result[3];
    matrices::transform_vectors(1, vector_at(body, 8), vector_at(car, 412), result);
    __movsd(reinterpret_cast<unsigned long *>(vector_at(car, 3160)), reinterpret_cast<const unsigned long *>(result), 3);
    matrices::transform_vectors(1, vector_at(at<void *>(car, 1768), 20), vector_at(car, 412),
                                result);
    body = at<void *>(car, 1768);
    __movsd(reinterpret_cast<unsigned long *>(vector_at(car, 3196)), reinterpret_cast<const unsigned long *>(result), 3);
    at<Dword>(car, 3160) = at<Dword>(car, 3168) = at<Dword>(car, 3200) = 0;
    matrices::transform_vectors(1, vector_at(car, 3160), vector_at(car, 376), result);
    __movsd(reinterpret_cast<unsigned long *>(vector_at(body, 8)), reinterpret_cast<const unsigned long *>(result), 3);
    body = at<void *>(car, 1768);
    matrices::transform_vectors(1, vector_at(car, 3196), vector_at(car, 376), result);
    __movsd(reinterpret_cast<unsigned long *>(vector_at(body, 20)), reinterpret_cast<const unsigned long *>(result), 3);
    for (unsigned offset : {3136u, 3140u, 3144u, 3172u, 3176u, 3180u})
        at<Dword>(car, offset) = 0;
}
} // namespace nfs::game::runtime::physics::car_frame
