#include "ai_profile_runtime.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include <cstdio>
namespace nfs::game::runtime::ai::lifecycle::pending {
namespace {
Dword *allocate_profile(Dword size, const char *label, const std::int32_t *offsets) {
    auto *profile =
        static_cast<Dword *>(nfs::lifecycle::allocate(size, label, nfs::lifecycle::level_pool));
    if (profile) {
        profile[0] = reinterpret_cast<Dword>(offsets);
        at<const Dword *>(profile, offsets[1]) = profile_base_methods;
    }
    return profile;
}
} // namespace

int load_racer_profiles(Dword *registry) {
    char filename[260];
    std::sprintf(filename, "%sAIRacers.ini", reinterpret_cast<char *>(bytes(system(4)) + 3644));
    if (!files::file::exists(filename))
        return -1;
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    int ordinal = 0;
    try {
        for (;;) {
            char section[32], type[64];
            std::sprintf(section, "entity%d", ordinal);
            type[0] = 0;
            ini::get_string(&reader, section, "type", type, 63);
            if (!type[0])
                break;
            Dword *profile = nullptr;
            void *interface = nullptr;
            switch (parse_entity_type(type)) {
            case 0:
                profile = allocate_profile(192, "AI::AIRacerProto", racer_profile_offsets);
                if (profile) {
                    construct_car_profile(profile, 0);
                    bind_profile_methods(profile, racer_base_methods, 164);
                    initialize_racer_traits(bytes(profile) + 160, 0, 2, 0, 0);
                    bind_profile_methods(profile, racer_profile_methods, 184);
                    profile[44] = 0;
                }
                break;
            case 1:
                profile = allocate_profile(172, "AI::AIPlayerProto", player_profile_offsets);
                if (profile) {
                    construct_car_profile(profile, 0);
                    bind_profile_methods(profile, racer_base_methods, 164);
                    bind_profile_methods(profile, player_profile_methods, 164);
                }
                break;
            case 2:
                profile = allocate_profile(172, "AI::PlayerCopProto", player_cop_profile_offsets);
                if (profile) {
                    construct_car_profile(profile, 0);
                    bind_profile_methods(profile, cop_profile_methods, 164);
                    bind_profile_methods(profile, player_cop_profile_methods, 164);
                }
                break;
            case 3:
                profile = allocate_profile(172, "AI::AICopProto", ai_cop_profile_offsets);
                if (profile) {
                    construct_car_profile(profile, 0);
                    bind_profile_methods(profile, cop_profile_methods, 164);
                    bind_profile_methods(profile, ai_cop_profile_methods, 164);
                }
                break;
            case 4:
                profile = allocate_profile(180, "AI::TrafficCarProto", traffic_profile_offsets);
                if (profile) {
                    construct_car_profile(profile, 0);
                    initialize_traffic_traits(bytes(profile) + 160, 0);
                    bind_profile_methods(profile, traffic_profile_methods, 172);
                }
                break;
            case 5:
                profile = static_cast<Dword *>(
                    nfs::lifecycle::allocate(192, "AI::PerpProto", nfs::lifecycle::level_pool));
                if (profile)
                    construct_perpetrator_profile(profile, 1);
                break;
            case 7:
                profile = static_cast<Dword *>(
                    nfs::lifecycle::allocate(12, "AI::Helicopter", nfs::lifecycle::level_pool));
                if (profile)
                    interface = construct_helicopter_profile(profile, 0);
                break;
            case 8:
                profile = static_cast<Dword *>(
                    nfs::lifecycle::allocate(8, "AI::SpikestripProto", nfs::lifecycle::level_pool));
                if (profile) {
                    profile[0] = reinterpret_cast<Dword>(profile_base_methods);
                    construct_spike_profile(profile);
                    profile[0] = reinterpret_cast<Dword>(spike_profile_methods);
                    interface = profile;
                }
                break;
            }
            if (profile && !interface)
                interface = bytes(profile) + reinterpret_cast<const int *>(profile[0])[1];
            if (interface) {
                invoke<Dword>(interface, 14, &reader, static_cast<const char *>(section));
                if (at<Dword>(registry, 180) < 45) {
                    registry[at<Dword>(registry, 180)] = reinterpret_cast<Dword>(interface);
                    ++at<Dword>(registry, 180);
                } else
                    invoke(interface, 1, Dword{1});
            }
            ++ordinal;
        }
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
    return ordinal;
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
