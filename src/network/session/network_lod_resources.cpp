#include "nfs/network/session/network_rules.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::network::rules::pending {
using namespace packets;
namespace {
// The call boundary prevents a raw DWORD copy from becoming an x87 F32 store.
__declspec(noinline) Dword scalar_bits(const float &value) {
    Dword bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}
} // namespace

void load_world_particle_lod(float *object, ini::Reader *reader, const char *section) {
    Dword period_bits = 1065353216, lifetime_bits = 1065353216, scale_bits = 1065353216;
    Dword near_bits = 1133903872; // farFadeIn defaults to 300.f.
    float value;
    if (ini::get_float(reader, section, "worldParticle.periodFactor", &value))
        period_bits = scalar_bits(value);
    if (ini::get_float(reader, section, "worldParticle.lifetimeFactor", &value))
        lifetime_bits = scalar_bits(value);
    if (ini::get_float(reader, section, "worldParticle.scaleFactor", &value))
        scale_bits = scalar_bits(value);
    if (ini::get_float(reader, section, "worldParticle.farFadeIn", &value))
        near_bits = scalar_bits(value);
    const long double far_distance =
        ini::get_float(reader, section, "worldParticle.farFadeOut", &value)
            ? static_cast<long double>(value)
            : 325.f;
    // Configuration copies keep their raw DWORDs; only the fade arithmetic loads F32 values.
    at<volatile Dword>(object, 0) = period_bits;
    at<volatile Dword>(object, 4) = lifetime_bits;
    at<volatile Dword>(object, 8) = scale_bits;
    auto *output = static_cast<volatile float *>(object);
    output[4] = static_cast<float>(far_distance);
    float near_distance;
    std::memcpy(&near_distance, &near_bits, sizeof(near_distance));
    const long double near_square = static_cast<long double>(near_distance) * near_distance;
    at<volatile Dword>(object, 12) = static_cast<volatile Dword &>(near_bits);
    output[5] = static_cast<float>(near_square);
    output[6] = static_cast<float>(far_distance * far_distance);
    const long double difference = static_cast<long double>(near_distance) - far_distance;
    const bool separated = difference != 0.f;
    output[7] = static_cast<float>(difference);
    if (separated)
        output[7] = static_cast<float>(1.f / difference);
}

void load_lod_section(Dword *object, ini::Reader *reader, const char *section) {
    Byte sun = 1, stencil = 1, splat = 1, generate_skids = 1, render_skids = 1, full_particles = 1,
         half_particles = 1, render_particles = 1;
    Dword minimum_force_bits = 1036831949, burnout_force_bits = 1056964608;
    float value;
    int integer;
    if (ini::get_int(reader, section, "sun.doRender", &integer))
        sun = integer != 0;
    if (ini::get_int(reader, section, "shadows.allowStencil", &integer))
        stencil = integer != 0;
    if (ini::get_int(reader, section, "shadows.allowSplat", &integer))
        splat = integer != 0;
    if (ini::get_int(reader, section, "skids.doGenerate", &integer))
        generate_skids = integer != 0;
    if (ini::get_float(reader, section, "skids.minForce", &value))
        minimum_force_bits = scalar_bits(value);
    if (ini::get_int(reader, section, "skids.doRender", &integer))
        render_skids = integer != 0;
    if (ini::get_int(reader, section, "carParticle.doGenerateFull", &integer))
        full_particles = integer != 0;
    if (ini::get_int(reader, section, "carParticle.doGenerateHalf", &integer))
        half_particles = integer != 0;
    if (ini::get_float(reader, section, "carParticle.burnoutForce", &value))
        burnout_force_bits = scalar_bits(value);
    if (ini::get_int(reader, section, "carParticle.doRender", &integer))
        render_particles = integer != 0;
    at<volatile Byte>(object, 4) = sun;
    at<volatile Byte>(object, 5) = stencil;
    at<volatile Byte>(object, 6) = splat;
    at<volatile Byte>(object, 7) = generate_skids;
    at<volatile Dword>(object, 8) = minimum_force_bits;
    at<volatile Byte>(object, 12) = render_skids;
    at<volatile Byte>(object, 13) = full_particles;
    at<volatile Byte>(object, 14) = half_particles;
    at<volatile Dword>(object, 16) = burnout_force_bits;
    at<volatile Byte>(object, 20) = render_particles;
    load_world_particle_lod(reinterpret_cast<float *>(object + 6), reader, section);
    if (!graphics::startup::stencil_enabled(&graphics::startup::settings, 0))
        at<Byte>(object, 5) = 0;
}

void reload_lod(Dword *object) {
    const Dword level = at<Dword>(system(72), 2388);
    char path[260];
    std::sprintf(path, "%sEAGL\\LOD.ini", reinterpret_cast<const char *>(system(4)) + 4);
    ini::Reader reader;
    ini::reader_construct(&reader, path, 0);
    const char *section;
    switch (level) {
    case 0:
        section = "off";
        break;
    case 1:
        section = "low";
        break;
    case 2:
        section = "med";
        break;
    case 4:
        section = "max";
        break;
    default:
        section = "high";
    }
    try {
        load_lod_section(object, &reader, section);
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}
} // namespace nfs::network::rules::pending
