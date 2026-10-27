#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cmath>
namespace nfs::game::runtime {
extern Byte particle_links[160], particle_states[3600], particle_geometry[55800];
namespace pending {
void *emit_particle(void *, const void *, float, const float *, const float *,
                             const float *, Dword, Dword);
extern Dword &particles_disabled;
void advance_emitter_clock_entry(void *, void *, float);
void position_emitter_sound_entry(Dword, Dword, const float *, const float *);
void advance_particle_emitter(void *);
} // namespace pending

void advance_particle_states(int listener) {
    kernel::memory_zero(particle_links + listener * 80, 80);
    for (unsigned i = 0; i < 50; ++i) {
        Byte *state = particle_states + listener * 1800 + i * 36;
        const int remaining = at<int>(state, 4);
        if (!remaining)
            continue;
        void *settings = at<void *>(state, 0);
        const int duration = at<int>(settings, 120);
        const unsigned opacity = at<Byte>(settings, 139);
        const int kind = at<int>(settings, 4);
        if (kind > 0 && kind < 20)
            ++at<Dword>(particle_links, (kind + listener * 20) * 4);
        at<Byte>(state, 15) = Byte(static_cast<int>(Dword(remaining) * opacity) / duration);
        --at<Dword>(state, 4);
        if (!at<Byte>(state, 15))
            at<Dword>(state, 4) = 0;
    }
}
namespace pending {

namespace {
// The depth is loaded as float before its sign changes. This separate load
// keeps the compiler from replacing FLD/FCHS with an integer sign-bit toggle.
__declspec(noinline) float load_emitter_sound_depth(const volatile float *depth) {
    return *depth;
}
}

void update_car_environment() {
    const Dword phase = (graphics::unresolved::camera_state >> 1) & 2;
    advance_particle_states(0);
    advance_particle_states(1);
    for (unsigned i = 0; i < 450; ++i) {
        Byte *particle = particle_geometry + i * 124;
        if (!at<std::int16_t>(particle, 4))
            continue;
        void *settings = at<void *>(particle, 0);
        if (!settings)
            continue;
        const float damping = at<float>(settings, 64);
        auto *velocity = reinterpret_cast<float *>(particle + 24);
        if (void *children = at<void *>(settings, 112)) {
            float direction[3] = {velocity[0], velocity[1], velocity[2]};
            const float zero[3]{};
            normalize_vector(direction, direction);
            for (int child = 0; child < at<int>(children, 8); ++child) {
                if (!emit_particle(children, particle + 12, 1, direction, zero, velocity,
                                            1, 0))
                    break;
                children = at<void *>(settings, 112);
            }
        }
        if (at<Dword>(settings, 68)) {
            const float rotation = at<float>(particle, 52);
            float magnitude = rotation;
            if (magnitude < 0)
                magnitude = -magnitude;
            const float position = at<float>(particle, 48);
            if (position > 0.25f)
                magnitude = -magnitude;
            else if (position >= -0.25f || std::isnan(position))
                magnitude = rotation;
            at<float>(particle, 52) = magnitude;
        }
        if (phase) {
            const float angular_speed = at<float>(particle, 60);
            if (angular_speed != 0) {
                float sine, cosine;
                turn_sine_cosine(reinterpret_cast<float *>(particle + 56), &sine, &cosine);
                velocity[0] =
                    float(static_cast<long double>(sine) * at<float>(particle, 64) + velocity[0]);
                velocity[2] = float(static_cast<long double>(cosine) * at<float>(particle, 64) +
                                    velocity[2]);
                const long double radial =
                    static_cast<long double>(damping) * at<float>(particle, 68);
                const long double angular =
                    static_cast<long double>(damping) * at<float>(particle, 60);
                at<float>(particle, 68) = float(radial);
                at<float>(particle, 60) = float(angular);
                at<float>(particle, 64) = float(radial + at<float>(particle, 64));
                at<float>(particle, 56) = float(angular + at<float>(particle, 56));
            }
            if (at<Dword>(settings, 88) != at<Dword>(settings, 84)) {
                const int age = at<std::int16_t>(particle, 6) - at<std::int16_t>(particle, 4);
                const long double blend = static_cast<long double>(age) * at<float>(particle, 8);
                const long double inverse = 1.0L - blend;
                for (int channel = 3; channel >= 0; --channel) {
                    const Dword start = Dword(static_cast<std::int64_t>(
                        at<Byte>(particle, 76 + channel) * inverse));
                    const Dword end = Dword(static_cast<std::int64_t>(
                        at<Byte>(particle, 80 + channel) * blend));
                    at<Byte>(particle, 72 + channel) = Byte(start + end);
                }
            }
        }
        at<float>(particle, 52) = damping * at<float>(particle, 52);
        scale_vectors(1, velocity, damping, velocity);
        --at<std::uint16_t>(particle, 4);
        if (at<Byte>(particle, 85) && !at<std::uint16_t>(particle, 4)) {
            const float zero[3]{};
            if (velocity[0] != 0 || velocity[1] != 0 || velocity[2] != 0) {
                float direction[3];
                normalize_vector(velocity, direction);
                for (unsigned child = 0; child < at<Byte>(particle, 85); ++child) {
                    const Dword value = emitter_random_word() & 65535;
                    const float scale =
                        float(static_cast<long double>(value) * 0.0000152587890625f * 0.75f + 0.25f);
                    if (void *spawned =
                            emit_particle(at<void *>(particle, 0), particle + 12, scale,
                                                   direction, zero, velocity, 0, 0)) {
                        at<std::int16_t>(spawned, 4) >>= 1;
                        at<Byte>(spawned, 85) = 0;
                    }
                }
            }
        }
    }
}

void update_world_particles(Dword *owner, Dword flags) {
    if (particles_disabled || !Byte(flags))
        return;
    const int count = at<int>(owner, 804);
    for (int i = 0; i < count; ++i)
        advance_particle_emitter(at<void *>(owner, 404 + i * 4));
}

void advance_particle_emitter(void *emitter) {
    if (!at<Byte>(emitter, 232))
        return;
    const float delta = float(static_cast<long double>(0.0333333350718021392822265625f) *
                              at<float>(emitter, 228));
    for (int i = 0; i < at<int>(emitter, 104); ++i)
        advance_emitter_clock_entry(bytes(emitter) + 108 + i * 24, emitter, delta);
    const float *source = at<const float *>(emitter, 32);
    float position[3];
    at<volatile Dword>(position, 0) = at<const volatile Dword>(source, 0);
    at<volatile Dword>(position, 4) = at<const volatile Dword>(source, 4);
    const float *direction = at<const float *>(emitter, 36);
    at<volatile Dword>(position, 8) = at<const volatile Dword>(source, 8);
    position[2] = -load_emitter_sound_depth(position + 2);
    float orientation[3];
    at<volatile Dword>(orientation, 0) = at<const volatile Dword>(direction, 0);
    at<volatile Dword>(orientation, 4) = at<const volatile Dword>(direction, 4);
    at<volatile Dword>(orientation, 8) = at<const volatile Dword>(direction, 8);
    position_emitter_sound_entry(at<Dword>(emitter, 248), at<Dword>(emitter, 252), position,
                                    orientation);
}
} // namespace pending
} // namespace nfs::game::runtime
