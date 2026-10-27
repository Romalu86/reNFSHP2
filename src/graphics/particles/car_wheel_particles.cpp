#include "../vehicles/car_wheel_effects_runtime.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "../../math/vectors/retained_vector_providers.hpp"

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Wheel particle placement requires native retained intermediates");
namespace nfs::game::world::camera {
void active_camera_position(void *, float *);
}
namespace nfs::game::runtime::physics::contacts_pending {

void emit_wheel_particles(void *car, int index, const float *position, int kind, float strength) {
    auto retain = [](float value) { return static_cast<long double>(value); };
    if (!audio::vehicle::car_audio_suppressed(car)) {
        float closest = -1;
        void *body = at<void *>(car, 1768);
        const int cameras = split_screen() ? 2 : 1;
        for (int camera = 0; camera < cameras; ++camera) {
            float eye[3];
            world::camera::active_camera_position(tick_services[camera], eye);
            long double distance;
            hardware::store_point_distance(eye, &at<float>(body, 680), &distance);
            if (distance < retain(closest) || closest < 0)
                closest = float(distance);
        }
        if (closest > 150)
            return;
    }
    void *definition = particles::lifecycle::particle_prefix;
    while (at<Dword>(definition, 4) && at<Dword>(definition, 4) != Dword(kind))
        definition = bytes(definition) + 152;
    if (!at<Dword>(definition, 4) || !kind || strength < 0)
        return;
    float point[3] = {position[0], position[1], position[2]};
    const Dword count = at<Dword>(definition, 8);
    if (strength > 1)
        strength = 1;
    else if (strength == 0)
        return;
    void *body = at<void *>(car, 1768);
    void *wheel = bytes(car) + 1948 + Dword(index) * 296;
    const float velocity[3] = {float(retain(at<float>(body, 8))), float(retain(at<float>(body, 12))),
                               float(-retain(at<float>(body, 16)))};
    float direction[3] = {float(-retain(at<float>(body, 716))), 0, at<float>(body, 724)};
    float angle = at<float>(wheel, 20);
    if (at<float>(wheel, 156) < 0) {
        direction[2] = float(-retain(direction[2]));
        direction[0] = float(-retain(direction[0]));
        const Dword ordinal = at<Dword>(wheel, 12);
        angle = float(retain(ordinal == 1 || ordinal == 2 ? -.125f : .125f) + angle);
    }
    const float sine = float(turn_sine(angle));
    const auto cosine = turn_cosine(angle);
    const auto prior_x = retain(direction[0]), prior_z = retain(direction[2]);
    const auto rotated_z = prior_z * cosine - retain(sine) * prior_x;
    const auto sine_z = retain(sine) * prior_z;
    const auto rotated_x = sine_z + cosine * prior_x;
    direction[0] = float(rotated_x);
    direction[2] = float(rotated_z);
    const bool left = index == 0 || index == 3;
    const float side[3] = {left ? float(-retain(at<float>(body, 692))) : at<float>(body, 692),
                           left ? float(-retain(at<float>(body, 696))) : at<float>(body, 696),
                           left ? at<float>(body, 700) : float(-retain(at<float>(body, 700)))};
    const float sign = left ? -1.f : 1.f;
    if (index < 2) {
        point[0] = float(retain(point[0]) - retain(at<float>(body, 8)) * 0.0333333350718021392822265625f);
        point[1] = float(retain(point[1]) - retain(at<float>(body, 12)) * 0.0333333350718021392822265625f);
        point[2] = float(retain(at<float>(body, 16)) * 0.0333333350718021392822265625f + point[2]);
    }
    for (Dword emitted = 0; emitted != count; ++emitted) {
        const Dword variant = kind == 1 || kind == 5 || kind == 13 || kind == 15;
        void *particle = particles::lifecycle::emit_car_particle(
            definition, point, strength, direction, side, velocity, 1, variant);
        if (particle) {
            at<float>(particle, 52) = float(retain(sign) * at<float>(particle, 52));
            at<float>(particle, 16) =
                float(retain(at<float>(particle, 36)) * .5f + at<float>(particle, 16));
        }
    }
}
} // namespace nfs::game::runtime::physics::contacts_pending
