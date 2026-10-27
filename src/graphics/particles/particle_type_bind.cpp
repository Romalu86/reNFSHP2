#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include <cmath>

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Particle ranges require native retained intermediates");
namespace nfs::game::runtime::particles {
namespace {
// A scalar x87 load quiets a signaling input before both lifetime stores.
__declspec(noinline) float load_lifetime(const volatile float *value) {
    return *value;
}
}
namespace pending {
Dword *find_fsh_image(Dword *, const char *);
}

Dword *find_texture(Dword *pool, const char *name) {
    Dword *archive = at<Dword *>(pool, 808);
    return archive ? pending::find_fsh_image(archive, name) : nullptr;
}
// MPB7 ranges, colour keys, damping and referenced FSH image.
void pending::initialize_type(Dword *object, Dword *owner, Dword *definition, Dword *pool) {
    reset_type(object);
    auto retain = [](float value) { return static_cast<long double>(value); };
    auto range = [&](unsigned offset, long double low, long double high) {
        at<float>(object, offset) = float(low);
        at<float>(object, offset + 4) = float(high);
        at<float>(object, offset + 8) = float(high - low);
    };
    if (definition[0] == 1) {
        const float lifetime = load_lifetime(reinterpret_cast<const float *>(definition + 1));
        at<volatile Dword>(object, 96) = 0;
        at<volatile float>(object, 88) = lifetime;
        at<volatile float>(object, 92) = lifetime;
    } else if (definition[0] == 2) {
        const auto half = retain(at<float>(definition, 8)) * 0.5f;
        const auto high = half + at<float>(definition, 4);
        const auto low = retain(at<float>(definition, 4)) - half;
        range(88, low, high);
    }
    object[25] = definition[21];
    const auto start_rotation_high = retain(at<float>(definition, 72)) * 0.00277777784503996372222900390625f;
    const auto start_rotation_low = retain(at<float>(definition, 68)) * 0.00277777784503996372222900390625f;
    range(172, start_rotation_low, start_rotation_high);
    const auto rotation_rate_high = retain(at<float>(definition, 80)) * 0.00277777784503996372222900390625f;
    const auto rotation_rate_low = retain(at<float>(definition, 76)) * 0.00277777784503996372222900390625f;
    range(184, rotation_rate_low, rotation_rate_high);
    for (unsigned key = 0; key < 3; ++key)
        for (unsigned channel = 0; channel < 4; ++channel)
            at<float>(object, 216 + key * 16 + channel * 4) =
                at<Byte>(definition, 88 + key * 4 + (channel + 1) % 4);
    for (unsigned i = 0; i < 4; ++i) {
        unsigned source = 36 + i * 8;
        const auto high = retain(at<float>(definition, source + 4));
        const auto low = retain(at<float>(definition, source));
        if (i < 2)
            range(40 + i * 12, low * 0.5f, high * 0.5f);
        else
            range(40 + i * 12, low, high);
    }
    if (at<Dword>(owner, 44) == 0)
        object[26] = 2;
    else if (at<Dword>(owner, 44) == 1)
        object[26] = 1;
    const auto frequency = retain(at<float>(owner, 48));
    const auto period = frequency != 0.0L
                            ? 1.0L / at<float>(owner, 48)
                            : retain(1000000.f);
    at<float>(object, 28) = float(period);
    object[9] = 0;
    at<float>(object, 32) = float(period);
    object[0] = definition[4];
    const auto spawn_high = retain(at<float>(definition, 24));
    const auto spawn_low = retain(at<float>(definition, 20));
    range(4, spawn_low, spawn_high);
    const auto spawn_random_high = retain(at<float>(definition, 32));
    const auto spawn_random_low = retain(at<float>(definition, 28));
    range(16, spawn_random_low, spawn_random_high);
    const auto owner_high = retain(at<float>(owner, 56));
    const auto owner_low = retain(at<float>(owner, 52));
    range(160, owner_low, owner_high);
    for (unsigned i = 0; i < 3; ++i)
        object[37 + i] = at<Dword>(owner, 60 + i * 4);
    normalize_vector(reinterpret_cast<float *>(object + 37),
                     reinterpret_cast<float *>(object + 37));
    at<float>(object, 144) = float(retain(at<float>(owner, 72)) * 0.5f);
    const auto half = retain(at<float>(owner, 80)) * 0.5f;
    const auto center = retain(at<float>(owner, 76));
    const float low = float(center - half);
    at<float>(object, 132) = low;
    const auto high = half + center;
    at<float>(object, 136) = float(high);
    at<float>(object, 140) = float(high - low);
    for (unsigned i = 0; i < 3; ++i)
        object[50 + i] = definition[25 + i];
    object[49] = 0;
    const auto damping_exponent = -(retain(0.0333333350718021392822265625f) * at<float>(definition, 112));
    const double compatible_exponent = double(damping_exponent);
    at<float>(object, 212) = float(std::exp(compatible_exponent));
    const char *texture = reinterpret_cast<const char *>(definition + 29);
    if (*texture)
        if (Dword *shape = find_texture(pool, texture)) {
            const Dword flags = at<Dword>(pool, 812);
            Dword *image = static_cast<Dword *>(lifecycle::allocate(48, "particleTex", flags));
            Dword *constructed_image;
            try {
                constructed_image = image ? graphics::tar::construct_shape(image, shape) : nullptr;
            } catch (...) {
                track::pending::release_named(image, "particleTex", flags);
                throw;
            }
            at<Dword *>(object, 196) = constructed_image;
        }
}

void pending::prepare_type(Dword *object) {
    auto f = [&](unsigned offset) { return reinterpret_cast<float *>(bytes(object) + offset); };
    subtract_vector4(f(232), f(216), f(264));
    add_vector4(f(264), f(264), f(264));
    subtract_vector4(f(248), f(232), f(280));
    add_vector4(f(280), f(280), f(280));
    add_vector4(f(232), f(232), f(296));
    subtract_vector4(f(296), f(248), f(296));
    for (unsigned i = 0; i < 3; ++i)
        at<float>(object, 312 + i * 4) =
            float(static_cast<long double>(at<float>(object, 120 + i * 4)) -
                  at<float>(object, 108 + i * 4));
    at<float>(object, 324) =
        float(1.0L - turn_cosine(float(static_cast<long double>(at<float>(object, 144)) * 0.5L)));
    scale_vectors(1, f(200), 0.0333333350718021392822265625f, f(328));
}
} // namespace nfs::game::runtime::particles
