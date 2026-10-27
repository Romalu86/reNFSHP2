#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include <cstdio>
#include <cstring>
#include <cmath>

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Particle definition conversion requires native retained intermediates");

namespace nfs::ini {
// Keep all four initial components for a partial sscanf conversion.
Dword get_colour(Reader *reader, const char *section, const char *key, std::uint8_t *output) {
    Key *entry = reader_key(reader, section, key);
    if (!entry || entry->value.length < 1 || !gui::string_data(&entry->value))
        return 0;
    const void *text = gui::string_data(&entry->value);
    if (entry->value.wide ? !*static_cast<const std::uint16_t *>(text)
                          : !*static_cast<const char *>(text))
        return 0;
    int alpha = output[3], red = output[2], green = output[1], blue = output[0];
    if (!std::sscanf(gui::string_ascii(&entry->value), "[%d,%d,%d,%d]", &alpha, &red, &green,
                     &blue))
        return 0;
    output[3] = std::uint8_t(alpha);
    output[0] = std::uint8_t(blue);
    output[2] = std::uint8_t(red);
    output[1] = std::uint8_t(green);
    return 1;
}
} // namespace nfs::ini
namespace nfs::game::runtime::particles::lifecycle::pending {
namespace {
struct EffectBinding {
    const char *section;
    Dword kind, render_state, splatter_state;
};
Dword truncate_particle_count(long double value) {
    return Dword(hardware::truncate_integer_native80(&value));
}

const EffectBinding effect_bindings[] = {
    {"emitter_dirt", 1, 7216576, 7216432},       {"emitter_sparks", 2, 7216504, 7216432},
    {"emitter_gravel", 3, 7216576, 7216432},     {"emitter_grass", 4, 7216576, 7216432},
    {"emitter_smoke", 5, 7216576, 7216432},      {"emitter_fire", 6, 7216504, 7216432},
    {"emitter_water", 7, 7216576, 7216432},      {"emitter_mud", 8, 7216576, 7216432},
    {"emitter_leaves", 9, 7216576, 7216432},     {"emitter_mulch", 10, 7216576, 7216432},
    {"emitter_glass", 11, 7216576, 7216432},     {"emitter_exhaust", 13, 7216576, 7216432},
    {"emitter_wood", 14, 7216576, 7216432},      {"emitter_crash_debris", 12, 7216576, 7216432},
    {"emitter_smokebomb", 15, 7216576, 7216432}, {"emitter_explosion", 16, 7216504, 7216432},
    {"emitter_burst", 17, 7216504, 7216432},     {"emitter_bombsparks", 18, 7216504, 7216432},
    {"emitter_wash", 19, 7216576, 7216432}};
void *find_effect_art(const char *name) {
    for (unsigned address = 7216648; address < 7217160; address += 16) {
        void *art = &state<Byte>(address);
        if (at<void *>(art, 8) && !std::strcmp(static_cast<const char *>(art), name))
            return art;
    }
    return nullptr;
}
void import_mpb_defaults(void *definition, const Dword *emitter, const Dword *type) {
    auto retain = [](float value) { return static_cast<long double>(value); };
    auto ef = [&](unsigned field) { return at<float>(emitter, field); };
    auto tf = [&](unsigned field) { return at<float>(type, field); };
    if (void *art = find_effect_art(reinterpret_cast<const char *>(type + 29)))
        at<void *>(definition, 108) = art;
    auto lifetime = retain(tf(4)) * 30.0f;
    if (!(1.0L < lifetime))
        lifetime = 1.0L;
    at<Dword>(definition, 12) = truncate_particle_count(lifetime);
    if (ef(48) != 0) {
        const auto period = retain(30.0f) / ef(48);
        at<Dword>(definition, 16) = truncate_particle_count(period);
    }
    const auto size = retain(tf(36)) * .5f;
    at<Dword>(definition, 36) = emitter[18];
    const auto initial_size = size * tf(52);
    at<float>(definition, 72) = float(initial_size);
    at<float>(definition, 76) = float(size * tf(64) - initial_size);
    if (tf(56) != 0)
        at<float>(definition, 80) = float((retain(tf(56)) - tf(52)) / tf(56));
    at<float>(definition, 56) = float(retain(tf(80)) * 0.0000925925924093462526798248291015625f);
    at<float>(definition, 28) = float(retain(ef(60)) * ef(76));
    at<float>(definition, 20) = float(retain(ef(64)) * ef(76));
    at<float>(definition, 24) = float(retain(ef(68)) * ef(76));
    if (ef(80) != 0 && ef(76) != 0) {
        auto variance = retain(ef(80)) / ef(76);
        if (variance < 0.0L)
            variance = -variance;
        at<float>(definition, 32) = float(variance);
    } else
        at<Dword>(definition, 32) = 0;
    auto number = retain(ef(48)) * 0.0333333350718021392822265625f;
    if (!(1.0L < number))
        number = 1.0L;
    at<Dword>(definition, 8) = truncate_particle_count(number);
    const auto exponent = retain(tf(112)) * -0.0333333350718021392822265625f;
    const Byte start_red = at<Byte>(type, 90);
    const Byte start_green = at<Byte>(type, 89);
    const Byte start_blue = at<Byte>(type, 88);
    at<Byte>(definition, 87) = start_blue;
    const Byte start_alpha = at<Byte>(type, 91);
    at<Byte>(definition, 86) = start_green;
    const Byte end_blue = at<Byte>(type, 96);
    at<Byte>(definition, 85) = start_red;
    const Byte end_green = at<Byte>(type, 97);
    at<Byte>(definition, 84) = start_alpha;
    const Byte end_red = at<Byte>(type, 98);
    at<Byte>(definition, 91) = end_blue;
    const Byte end_alpha = at<Byte>(type, 99);
    at<Byte>(definition, 90) = end_green;
    const Dword vortex = type[21];
    at<Dword>(definition, 104) = 1125515264;
    at<Byte>(definition, 89) = end_red;
    at<Byte>(definition, 88) = end_alpha;
    at<Dword>(definition, 60) = vortex;
    // The exponential uses the binary64 library interface.
    at<float>(definition, 64) = float(std::exp(double(exponent)));
    at<float>(definition, 52) = float(retain(tf(104)) * 0.0333333350718021392822265625f);
}
void read_effect_mpb(void *definition, const Dword *image, const char *name) {
    if (*image != 7)
        return;
    const Dword *emitter = image + 3 + image[1] * 31;
    for (int index = 0; index < int(image[2]); ++index) {
        const Dword *references = emitter + 26;
        for (Dword child = 0; child < emitter[24]; ++child) {
            const Dword *type = image + 3 + references[child] * 31;
            if (!std::strcmp(reinterpret_cast<const char *>(emitter), name)) {
                import_mpb_defaults(definition, emitter, type);
                return;
            }
        }
        emitter = references + emitter[24] + emitter[25];
    }
}
} // namespace

void load_car_particle_definitions() {
    char configuration_path[260], particle_path[260];
    const char *directory = reinterpret_cast<const char *>(system(4)) + 4;
    std::sprintf(configuration_path, "%sParticle\\carfx.ini", directory);
    if (!files::file::exists(configuration_path))
        return;
    std::sprintf(particle_path, "%sParticle\\particle.mpb", directory);
    if (!files::file::exists(particle_path))
        return;
    auto *image = static_cast<Dword *>(
        files::file::try_load(particle_path, nfs::lifecycle::cars_pool | 1024));
    if (!image)
        return;
    ini::Reader reader;
    ini::reader_construct(&reader, configuration_path, 0);
    try {
        for (const auto &binding : effect_bindings) {
            int &count = state<int>(7217168);
            if (count >= 32)
                break;
            void *definition = &state<Byte>(7151864 + Dword(count) * 152);
            char art_name[256]{};
            read_effect_mpb(definition, image, binding.section);
            ini::get_string(&reader, binding.section, "art", art_name, 255);
            if (void *art = find_effect_art(art_name))
                at<void *>(definition, 108) = art;
            if (!at<void *>(definition, 108))
                continue;
            at<const char *>(definition, 0) = binding.section;
            at<Dword>(definition, 4) = binding.kind;
            at<void *>(definition, 96) = &state<Byte>(binding.render_state);
            auto integer = [&](const char *key, unsigned field) {
                ini::get_int(&reader, binding.section, key, &at<int>(definition, field));
            };
            auto real = [&](const char *key, unsigned field) {
                ini::get_float(&reader, binding.section, key, &at<float>(definition, field));
            };
            auto colour = [&](const char *key, unsigned field) {
                ini::get_colour(&reader, binding.section, key, &at<Byte>(definition, field));
            };
            integer("number", 8);
            integer("life", 12);
            integer("tickDelay", 16);
            real("speedY", 20);
            real("speedZ", 24);
            real("speedX", 28);
            real("speedVariance", 32);
            real("spread", 36);
            real("spinV", 40);
            real("spinR", 44);
            real("spinRV", 48);
            real("gravity", 52);
            real("rotation", 56);
            real("vortex", 60);
            real("drag", 64);
            integer("bottomHeavy", 68);
            real("streaky", 100);
            real("size", 72);
            real("growth", 76);
            real("variance", 80);
            colour("c_start", 84);
            colour("c_end", 88);
            colour("c_variance", 92);
            real("renderDist", 104);
            art_name[0] = 0;
            ini::get_string(&reader, binding.section, "splatterArt", art_name, 255);
            if (void *art = find_effect_art(art_name)) {
                at<void *>(definition, 144) = art;
                at<void *>(definition, 124) = &state<Byte>(binding.splatter_state);
                integer("numSplatters", 116);
                integer("splatterLife", 120);
                real("splatterMaxSize", 128);
                real("splatterVariance", 132);
                colour("splatterARGB", 136);
                real("splatterRandom", 140);
                integer("splatterMaxPerPlayer", 148);
            } else
                at<Dword>(definition, 144) = 0;
            if (binding.kind == 5 || binding.kind == 1) {
                const float growth = at<float>(definition, 76);
                if (!(growth < 1.5f) || std::isnan(growth))
                    at<float>(definition, 76) = 1.5f;
            }
            ++count;
        }
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    kernel::memory_free(image);
    ini::reader_clear(&reader);
}
} // namespace nfs::game::runtime::particles::lifecycle::pending
