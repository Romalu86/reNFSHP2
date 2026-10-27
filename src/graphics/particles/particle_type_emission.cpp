#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include <cmath>
namespace nfs::lifecycle::construction {
extern Dword particle_head,
    particle_tail;
}
namespace nfs::game::runtime::particles::lifecycle {
namespace {
// Keep captured parameter/settings words in integer registers until their
// original numeric use; an inlined bitcast can become an early floating load.
__declspec(noinline) Dword captured_word(const void *value) {
    return *static_cast<const volatile Dword *>(value);
}
}

void *allocate_type_particle(void *buffer, void *definition) {
    const int count = at<int>(buffer, 72000);
    if (count > 999)
        return nullptr;
    void *particle = at<void *>(buffer, 68000u + Dword(count) * 4);
    at<void *>(particle, 60) = definition;
    at<Dword>(buffer, 72000) = Dword(count) + 1;
    return particle;
}
namespace pending {
// Allocation precedes the lifetime rejection, as in the entry.
void emit_type_particle(void *definition, void *emitter, const float *inherited_velocity,
                        float age) {
    void *head = reinterpret_cast<void *>(nfs::lifecycle::construction::particle_head);
    auto *particle = static_cast<float *>(allocate_type_particle(at<void *>(head, 4), definition));
    if (!particle)
        return;
    // This TU retains the arithmetic in x87 precision; only FST32 boundaries below narrow values to
    // float.
    auto X = [](float value) { return static_cast<long double>(value); };
    auto settings = [] {
        return reinterpret_cast<void *>(nfs::lifecycle::construction::particle_tail);
    };
    auto random_fraction = [] {
        return static_cast<long double>(emitter_random_word() & 65535) * 0.0000152587890625f;
    };
    auto randomized = [&](unsigned base, unsigned range) {
        return random_fraction() * at<float>(definition, range) + at<float>(definition, base);
    };
    const Dword lifetime_scale = captured_word(bytes(settings()) + 4);
    const auto lifetime = randomized(88, 96) * at<const float>(&lifetime_scale, 0);
    const Dword age_word = captured_word(&age);
    if (!(X(at<const float>(&age_word, 0)) < lifetime))
        return;
    const long double reciprocal_lifetime = X(1) / lifetime;
    at<volatile Dword>(particle, 12) = age_word;
    const Dword direction_mode = at<volatile Dword>(definition, 104);
    particle[7] = float(reciprocal_lifetime);
    float *velocity = particle + 4;
    float transformed[3];
    if (!direction_mode) {
        velocity[0] = float(randomized(108, 312));
        velocity[1] = float(randomized(112, 316));
        const Dword vertical_random = emitter_random_word() & 65535;
        const float *transform = at<const float *volatile>(emitter, 40);
        velocity[2] = float(static_cast<long double>(vertical_random) * 0.0000152587890625f *
                                at<float>(definition, 320) +
                            at<float>(definition, 116));
        matrices::transform_vectors(1, velocity, transform, transformed);
        const Dword transformed_x = at<const volatile Dword>(transformed, 0);
        const Dword transformed_y = at<const volatile Dword>(transformed, 4);
        at<volatile Dword>(velocity, 0) = transformed_x;
        const Dword transformed_z = at<const volatile Dword>(transformed, 8);
        at<volatile Dword>(velocity, 4) = transformed_y;
        const long double inherited_x = X(inherited_velocity[0]);
        at<volatile Dword>(velocity, 8) = transformed_z;
        velocity[0] = float(inherited_x + velocity[0]);
        velocity[1] += inherited_velocity[1];
        velocity[2] += inherited_velocity[2];
        scale_vectors(1, velocity, particle[3], particle);
    } else {
        float direction[3];
        if (direction_mode == 2) {
            spread_direction(reinterpret_cast<const float *>(bytes(definition) + 148),
                             at<float>(definition, 324), direction);
        } else {
            const float turns = float(random_fraction());
            const Dword height_random = emitter_random_word() & 65535;
            const auto half_height = static_cast<long double>(height_random) * 0.0000152587890625f;
            const auto extended_height = half_height + half_height - 1.f;
            const float height = float(extended_height);
            const float radial_squared = float(X(1) - extended_height * height);
            const float radius = float(std::sqrt(static_cast<long double>(radial_squared)));
            direction[0] = float(turn_cosine(turns) * radius);
            direction[1] = float(turn_sine(turns) * radius);
            at<Dword>(direction, 8) = at<const Dword>(&height, 0);
        }
        scale_vectors(1, direction, float(randomized(132, 140)), velocity);
        matrices::transform_vectors(1, velocity, at<const float *>(emitter, 40), transformed);
        const Dword transformed_x = at<const volatile Dword>(transformed, 0);
        const Dword transformed_y = at<const volatile Dword>(transformed, 4);
        at<volatile Dword>(velocity, 0) = transformed_x;
        const Dword transformed_z = at<const volatile Dword>(transformed, 8);
        at<volatile Dword>(velocity, 4) = transformed_y;
        at<volatile Dword>(velocity, 8) = transformed_z;
        velocity[0] += inherited_velocity[0];
        velocity[1] += inherited_velocity[1];
        velocity[2] += inherited_velocity[2];
        float displacement[3];
        scale_vectors(1, direction, float(randomized(160, 168)), displacement);
        scale_vectors(1, velocity, particle[3], particle);
        particle[0] = displacement[0] + particle[0];
        particle[1] = displacement[1] + particle[1];
        particle[2] = displacement[2] + particle[2];
    }
    const long double particle_x = X(particle[0]);
    const float *position = at<const float *volatile>(emitter, 32);
    particle[0] = float(particle_x + position[0]);
    particle[1] = position[1] + particle[1];
    particle[2] = position[2] + particle[2];
    particle[12] = float(randomized(172, 180));
    particle[13] = float(randomized(184, 192));
    const Dword size_random = emitter_random_word() & 65535;
    const Dword width_scale = captured_word(bytes(settings()) + 8);
    const float size = float(static_cast<long double>(size_random) * 0.0000152587890625f *
                                 at<float>(definition, 72) +
                             at<float>(definition, 64));
    const Dword width_random = emitter_random_word() & 65535;
    const Dword height_scale = captured_word(bytes(settings()) + 8);
    const float width = float((static_cast<long double>(width_random) * 0.0000152587890625f *
                                   at<float>(definition, 48) +
                               at<float>(definition, 40)) *
                              at<const float>(&width_scale, 0) * size);
    particle[8] = width;
    const float height = float(randomized(52, 60) * at<const float>(&height_scale, 0) * size);
    particle[9] = height;
    const auto variation = randomized(76, 84) - size;
    particle[10] = float(X(width) * variation);
    particle[11] = float(X(height) * variation);
}
} // namespace pending
} // namespace nfs::game::runtime::particles::lifecycle
