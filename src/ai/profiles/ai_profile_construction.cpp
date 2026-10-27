#include "ai_profile_runtime.hpp"
#include <cstring>
namespace nfs::game::runtime::ai::lifecycle::pending {

const std::int32_t car_profile_offsets[] = {0, 164};
const std::int32_t racer_profile_offsets[] = {0, 184};
const std::int32_t player_profile_offsets[] = {0, 164};
const std::int32_t player_cop_profile_offsets[] = {0, 164};
const std::int32_t ai_cop_profile_offsets[] = {0, 164};
const std::int32_t traffic_profile_offsets[] = {0, 172};
const std::int32_t perpetrator_profile_offsets[] = {0, 184};

Dword *construct_car_profile(Dword *profile, int virtual_base) {
    if (virtual_base) {
        profile[0] = reinterpret_cast<Dword>(car_profile_offsets);
        profile[41] = reinterpret_cast<Dword>(profile_base_methods);
    }
    profile[37] = Dword(-1);
    profile[39] = Dword(-1);
    profile[38] = 0;
    bind_profile_methods(profile, car_profile_methods, 164);
    std::memset(profile + 1, 0, 112);
    at<Byte>(profile, 148) = 0;
    at<Byte>(profile, 116) = 0;
    return profile;
}

void *initialize_racer_traits(void *traits, Dword skill, Dword colour, Dword attitude, Dword style) {
    at<std::uint16_t>(traits, 0) = std::uint16_t(skill);
    at<Dword>(traits, 4) = colour;
    at<Dword>(traits, 8) = attitude;
    at<Dword>(traits, 12) = style;
    return traits;
}

void *initialize_traffic_traits(void *traits, Dword value) {
    at<Dword>(traits, 0) = value;
    at<Dword>(traits, 4) = 0;
    return traits;
}
// Net construction first installs the saved Racer table.
Dword *construct_net_profile(Dword *profile, int virtual_base) {
    if (virtual_base) {
        profile[0] = reinterpret_cast<Dword>(racer_profile_offsets);
        profile[46] = reinterpret_cast<Dword>(profile_base_methods);
    }
    construct_car_profile(profile, 0);
    bind_profile_methods(profile, racer_base_methods, 164);
    initialize_racer_traits(bytes(profile) + 160, 0, 2, 0, 0);
    bind_profile_methods(profile, racer_profile_methods, 184);
    profile[44] = 0;
    return profile;
}

Dword *construct_perpetrator_profile(Dword *profile, int virtual_base) {
    if (virtual_base) {
        profile[0] = reinterpret_cast<Dword>(perpetrator_profile_offsets);
        profile[46] = reinterpret_cast<Dword>(profile_base_methods);
    }
    construct_car_profile(profile, 0);
    bind_profile_methods(profile, racer_base_methods, 164);
    initialize_racer_traits(bytes(profile) + 160, 0, 2, 0, 0);
    bind_profile_methods(profile, racer_profile_methods, 184);
    profile[44] = 0;
    bind_profile_methods(profile, perpetrator_profile_methods, 184);
    return profile;
}

Dword *construct_helicopter_profile(Dword *profile, Dword value) {
    profile[0] = reinterpret_cast<Dword>(profile_base_methods);
    profile[2] = value;
    profile[0] = reinterpret_cast<Dword>(helicopter_profile_methods);
    return profile;
}

Dword *copy_helicopter_profile(Dword *profile, const Dword *source) {
    profile[1] = source[1];
    profile[0] = reinterpret_cast<Dword>(profile_base_methods);
    profile[2] = source[2];
    profile[0] = reinterpret_cast<Dword>(helicopter_profile_methods);
    return profile;
}

Dword *assign_helicopter_profile_identifier(Dword *profile, const Dword *source) {
    if (profile != source)
        profile[1] = source[1];
    return profile;
}
// Complete RET leaf.
void construct_spike_profile(void *) {
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
