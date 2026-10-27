#include "ai_profile_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace nfs::game::runtime::ai::lifecycle::pending {
namespace {
struct ProfileToken {
    Dword value;
    const char *name;
};

const ProfileToken styles[] = {{1, "defaultStyle"}, {Dword(-1), nullptr}};
const ProfileToken attitudes[] = {{1, "timid"},      {2, "cautious"}, {3, "neutral"},
                                  {4, "aggressive"}, {5, "angry"},    {Dword(-1), nullptr}};
Dword profile_token(const ProfileToken *token, const char *text) {
    do {
        if (!std::strcmp(token->name, text))
            return token->value;
        ++token;
    } while (token->value != Dword(-1));
    return 0;
}
float saved_profile_float(Dword bits) {
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
float profile_speed(const char *text) {
    const auto format = kernel::numeric::current_format();
    return float(kernel::numeric::Extended::from_double(std::atof(text), format) *
                 kernel::numeric::Extended(0.44703f, format)); // Multiply by the single-precision coefficient.
}
} // namespace
// Ten case-sensitive profile names.
int parse_entity_type(const char *name) {
    static const char *const kinds[] = {
        "Racer", "Player", "PlayerCop",  "Cop",        "Traffic",
        "Perp",  "NetCar", "Helicopter", "Spikestrip", "kHeliProjectile,"};
    for (int i = 0; i < 10; ++i)
        if (!std::strcmp(name, kinds[i]))
            return i;
    return -1;
}
// Four-word profile records.
Dword parse_profile_colour(const char *name) {
    const ProfileColour *colour = profile_colours;
    do {
        if (!std::strcmp(colour->name, name))
            return colour->value;
        ++colour;
    } while (colour->value != Dword(-1));
    return Dword(-1);
}

Dword __fastcall load_profile_identifier(void *profile, void *, ini::Reader *, const char *section) {
    at<Dword>(profile, 4) = Dword(-1);
    char copied_section[256];
    std::strcpy(copied_section, section);
    int identifier = 0;
    if (std::sscanf(section, "entity%d", &identifier) == 1)
        at<int>(profile, 4) = identifier;
    return 0;
}
// The receiver is the Car virtual base, not the complete profile.
Dword __fastcall load_car_profile(void *profile, void *, ini::Reader *reader, const char *section) {
    void *complete_profile = bytes(profile) - 164;
    void *identity = bytes(complete_profile) + at<const int *>(complete_profile, 0)[1];
    load_profile_identifier(identity, nullptr, reader, section);
    std::strncpy(reinterpret_cast<char *>(bytes(profile) - 48), "", 32);
    profile_field<Dword>(profile, -16) = 15;
    profile_field<Dword>(profile, -12) = 0;
    char value[256]{};
    if (ini::get_string(reader, section, "name", value, 255) & 255)
        std::strncpy(reinterpret_cast<char *>(bytes(profile) - 48), value, 32);
    std::memset(value, 0, 255);
    if (ini::get_string(reader, section, "car", value, 255) & 255) {
        const dependencies::CarName *car = dependencies::car_names;
        Dword model = Dword(-1);
        do {
            if (!std::strcmp(car->name, value)) {
                model = Dword(car->model);
                break;
            }
            ++car;
        } while (car->model != -1);
        profile_field<Dword>(profile, -16) = model;
    }
    std::memset(value, 0, 255);
    if (ini::get_string(reader, section, "skin", value, 255) & 255) {
        const dependencies::SkinName *skin = dependencies::skin_names;
        Dword variant = 0;
        do {
            if (!std::strcmp(value, skin->name)) {
                variant = Dword(skin->variant);
                break;
            }
            ++skin;
        } while (skin->variant != -1);
        profile_field<Dword>(profile, -12) = variant;
    }
    const char *const keys[] = {"HonkBlocking",   "HonkOnComming",  "HonkAngry",
                                "HonkCollision1", "HonkCollision2", "HonkCollision3",
                                "HonkCollision4"};
    for (int i = 0; i < 7; ++i) {
        char *horn = reinterpret_cast<char *>(bytes(profile) - 160 + i * 16);
        std::memset(horn, 0, 15);
        ini::get_string(reader, section, keys[i], horn, 15);
    }
    return 3;
}

Dword __fastcall load_racer_profile(void *profile, void *, ini::Reader *reader,
                                   const char *section) {
    load_car_profile(bytes(profile) - 20, nullptr, reader, section);
    profile_field<Dword>(profile, -20) = 2;
    profile_field<std::uint16_t>(profile, -24) = 0;
    profile_field<Dword>(profile, -12) = profile_field<Dword>(profile, -16) = 0;
    char value[256]{};
    if (ini::get_string(reader, section, "skill", value, 255) & 255)
        profile_field<std::uint16_t>(profile, -24) = std::uint16_t(std::atoi(value));
    std::memset(value, 0, 255);
    if (ini::get_string(reader, section, "style", value, 255) & 255)
        profile_field<Dword>(profile, -12) = profile_token(styles, value);
    std::memset(value, 0, 255);
    if (ini::get_string(reader, section, "attitude", value, 255) & 255)
        profile_field<Dword>(profile, -16) = profile_token(attitudes, value);
    std::memset(value, 0, 255);
    if (ini::get_string(reader, section, "color", value, 255) & 255)
        profile_field<Dword>(profile, -20) = parse_profile_colour(value);
    return 4;
}

Dword __fastcall load_traffic_profile(void *profile, void *, ini::Reader *reader,
                                     const char *section) {
    load_car_profile(bytes(profile) - 8, nullptr, reader, section);
    profile_field<float>(profile, -12) = saved_profile_float(1103409526);
    char value[256]{};
    if (ini::get_string(reader, section, "speed", value, 255) & 255)
        profile_field<float>(profile, -12) = profile_speed(value);
    ini::get_float(reader, section, "delay", &profile_field<float>(profile, -8));
    return 1;
}

Dword __fastcall load_helicopter_profile(void *profile, void *, ini::Reader *reader,
                                        const char *section) {
    load_profile_identifier(profile, nullptr, reader, section);
    char value[256]{};
    at<float>(profile, 8) = saved_profile_float(1124473832);
    if (ini::get_string(reader, section, "maxspeed", value, 255) & 255)
        at<float>(profile, 8) = profile_speed(value);
    return 1;
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
