#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cmath>
#include <cstring>

namespace nfs::game::runtime::particles::lifecycle {

void perpendicular_vector(const float *direction, float *output) {
    float absolute = direction[0];
    if (absolute < 0)
        absolute = -absolute;
    if (absolute < 0.002f) {
        output[0] = 1;
        output[1] = output[2] = 0;
        return;
    }
    const float x = direction[0];
    auto E = [](float value) { return static_cast<long double>(value); };
    const auto yy = E(direction[1]) * direction[1];
    const auto xx = E(direction[0]) * direction[0];
    const float squared = float(yy + xx);
    const float magnitude = float(__builtin_sqrtl(E(squared)));
    const auto inverse = E(1) / E(magnitude);
    output[2] = 0;
    output[0] = float(inverse * direction[1]);
    output[1] = float(-(inverse * x));
}

void normalized_cross(const float *first, const float *second, float *output) {
    float cross[3];
    cross_vectors(first, second, cross);
    normalize_vector(cross, output);
}

void spread_direction(const float *direction, float spread, float *output) {
    auto E = [](float value) { return static_cast<long double>(value); };
    float perpendicular[3], cross[3], side[3];
    perpendicular_vector(direction, perpendicular);
    normalized_cross(direction, perpendicular, cross);
    const Dword azimuth = emitter_random_word() & 65535;
    const float turns = static_cast<float>(azimuth) * 0.0000152587890625f;
    const float sine = static_cast<float>(turn_sine(turns));
    const float cosine = static_cast<float>(turn_cosine(turns));
    scale_vectors(1, perpendicular, cosine, side);
    scale_vectors(1, cross, sine, cross);
    for (unsigned axis = 0; axis < 3; ++axis)
        side[axis] = float(E(cross[axis]) + side[axis]);
    const Dword radius = emitter_random_word() & 65535;
    const auto retained_longitudinal =
        E(1) - static_cast<long double>(radius) * 0.0000152587890625f * spread;
    const float longitudinal = float(retained_longitudinal);
    const float radial_squared = float(E(1) - retained_longitudinal * longitudinal);
    const float transverse = float(__builtin_sqrtl(E(radial_squared)));
    scale_vectors(1, direction, longitudinal, output);
    scale_vectors(1, side, transverse, side);
    for (unsigned axis = 0; axis < 3; ++axis)
        output[axis] = float(E(side[axis]) + output[axis]);
}

void *emit_car_particle(void *definition, const void *position, float scale, const float *direction,
                        const float *lateral, const float *inherited, Dword paced, Dword flags) {
    if (!definition || !at<void *>(definition, 108) ||
        !at<Dword>(at<void *>(definition, 108), 8) || !at<Dword>(definition, 96))
        return nullptr;
    const int phase = static_cast<int>(graphics::unresolved::camera_state) >> 2;
    const int period = static_cast<int>(at<Dword>(definition, 16) + 1);
    const bool alternate = (phase / static_cast<int>(static_cast<Dword>(period) * 2) *
                                static_cast<int>(static_cast<Dword>(period) * 2) ==
                            phase);
    const void *configuration = system(140);
    if (!at<Byte>(configuration, 13) && (!at<Byte>(configuration, 14) || alternate))
        return nullptr;
    if (paced && phase % period != 0)
        return nullptr;
    Byte *selected = nullptr;
    for (unsigned index = 0; index < 450; ++index) {
        Byte *candidate = &state<Byte>(7160496 + index * 124);
        const auto remaining = at<std::int16_t>(candidate, 4);
        if (!remaining) {
            selected = candidate;
            break;
        }
        if (!selected || remaining <= at<std::int16_t>(selected, 4))
            selected = candidate;
    }
    if (!selected)
        selected = &state<Byte>(7160496);
    const auto lifetime = at<std::int16_t>(definition, 12);
    *reinterpret_cast<volatile std::int16_t *>(selected + 4) = lifetime;
    *reinterpret_cast<volatile std::int16_t *>(selected + 6) = lifetime;
    at<Byte>(selected, 84) = alternate;
    const auto *source_position = static_cast<const volatile Dword *>(position);
    auto *destination_position = reinterpret_cast<volatile Dword *>(selected + 12);
    const Dword position_x = source_position[0];
    destination_position[0] = position_x;
    const Dword position_y = source_position[1];
    const Dword position_z = source_position[2];
    destination_position[1] = position_y;
    at<void *>(selected, 0) = definition;
    destination_position[2] = position_z;
    at<float>(selected, 8) = static_cast<float>(1.0L / lifetime);
    const Dword vertical_random = emitter_random_word() & 65535;
    const Dword forward_random = emitter_random_word() & 65535;
    const float forward =
        static_cast<float>(((static_cast<long double>(forward_random) * 0.0000152587890625f - 0.5L) *
                                at<float>(definition, 32) +
                            1.0L) *
                           at<float>(definition, 24));
    const Dword lateral_random = emitter_random_word() & 65535;
    const long double lateral_speed =
        ((static_cast<long double>(lateral_random) * 0.0000152587890625f - 0.5L) *
             at<float>(definition, 32) +
         1.0L) *
        at<float>(definition, 28);
    at<float>(selected, 24) = static_cast<float>(lateral_speed * lateral[0] * scale);
    at<float>(selected, 32) = static_cast<float>(lateral_speed * lateral[2] * scale);
    float heading[3] = {direction[0], direction[1], direction[2]};
    at<float>(selected, 28) =
        static_cast<float>(lateral_speed * lateral[1] * scale +
                           ((static_cast<long double>(vertical_random) * 0.0000152587890625f - 0.5L) *
                                at<float>(definition, 32) +
                            1.0L) *
                               at<float>(definition, 20) * scale);
    if (at<float>(definition, 36) != 0)
        spread_direction(heading, at<float>(definition, 36) * 0.5f, heading);
    for (unsigned axis = 0; axis < 3; ++axis)
        at<float>(selected, 24 + axis * 4) = static_cast<float>(
            static_cast<long double>(heading[axis]) * forward * scale +
            static_cast<long double>(inherited[axis]) * at<float>(definition, 60) +
            at<float>(selected, 24 + axis * 4));
    Dword random = emitter_random_word() & 65535;
    const auto size_fraction = static_cast<long double>(random) * 0.0000152587890625f;
    const auto size_variation = static_cast<long double>(at<float>(definition, 80)) *
                                at<float>(definition, 72);
    const auto size = size_fraction * size_variation + at<float>(definition, 72);
    *reinterpret_cast<volatile float *>(selected + 36) = static_cast<float>(size);
    *reinterpret_cast<volatile float *>(selected + 40) = static_cast<float>(size);
    at<float>(selected, 44) = static_cast<float>(
        static_cast<long double>(scale) * at<float>(definition, 76) * at<float>(selected, 8));
    random = emitter_random_word() & 65535;
    at<float>(selected, 48) = static_cast<float>(random) * 0.0000152587890625f;
    at<float>(selected, 52) = scale * at<float>(definition, 56);
    random = emitter_random_word() & 65535;
    at<Dword>(selected, 60) = at<Dword>(definition, 40);
    at<Dword>(selected, 64) = at<Dword>(definition, 44);
    at<float>(selected, 56) = static_cast<float>(random) * 0.0000152587890625f;
    const Dword final_colour = at<Dword>(definition, 88);
    const Dword particle_flags = at<Dword>(definition, 48);
    const Dword initial_colour = at<Dword>(definition, 84);
    at<Dword>(selected, 80) = final_colour;
    const Dword colour_variation = at<Dword>(definition, 92);
    at<Dword>(selected, 68) = particle_flags;
    at<Byte>(selected, 85) = 0;
    at<Dword>(selected, 76) = initial_colour;
    if (colour_variation)
        for (int channel = 3; channel >= 0; --channel) {
            random = emitter_random_word() & 65535;
            const auto subtract = static_cast<std::int64_t>(
                static_cast<long double>(random) * 0.0000152587890625f *
                at<Byte>(definition, 92 + channel));
            at<Byte>(selected, 76 + channel) =
                static_cast<Byte>(Dword(at<Byte>(selected, 76 + channel)) - Dword(subtract));
        }
    if (flags & 1) {
        at<Byte>(selected, 79) = static_cast<Byte>(
            static_cast<std::int64_t>(static_cast<long double>(at<Byte>(selected, 79)) * scale));
        at<Byte>(selected, 83) = static_cast<Byte>(
            static_cast<std::int64_t>(static_cast<long double>(at<Byte>(selected, 83)) * scale));
    }
    at<Dword>(selected, 72) = at<Dword>(selected, 76);
    std::memcpy(selected + 88, selected + 12, 12);
    std::memcpy(selected + 100, selected + 12, 12);
    at<Byte>(selected, 112) = at<Byte>(selected, 113) = 0;
    return selected;
}
} // namespace nfs::game::runtime::particles::lifecycle
namespace nfs::game::runtime::pending {
void *emit_particle(void *definition, const void *position, float scale,
                             const float *direction, const float *lateral, const float *inherited,
                             Dword paced, Dword flags) {
    return particles::lifecycle::emit_car_particle(definition, position, scale, direction, lateral,
                                                   inherited, paced, flags);
}
} // namespace nfs::game::runtime::pending
