#pragma once
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/ai/lifecycle/ai_dependencies.hpp"
#include "nfs/resources/ini/ini.hpp"
#include <initializer_list>
namespace nfs::game::runtime::ai::lifecycle::pending {
template <class T> T &profile_field(void *profile, int offset) {
    return *reinterpret_cast<T *>(bytes(profile) + offset);
}
inline void *adjust_profile_interface(void *profile) {
    return bytes(profile) - profile_field<int>(profile, -4);
}
inline void bind_profile_methods(void *object, const Dword *methods, int base_size) {
    const int offset = reinterpret_cast<const int *>(at<Dword>(object, 0))[1];
    at<const Dword *>(object, offset) = methods;
    at<int>(object, offset - 4) = offset - base_size;
}
extern const Dword racer_profile_methods[], racer_base_methods[], car_profile_methods[],
    perpetrator_profile_methods[], player_profile_methods[], cop_profile_methods[],
    player_cop_profile_methods[], ai_cop_profile_methods[], traffic_profile_methods[],
    helicopter_profile_methods[], spike_profile_methods[], projectile_profile_methods[];
extern const std::int32_t car_profile_offsets[], racer_profile_offsets[], player_profile_offsets[],
    player_cop_profile_offsets[], ai_cop_profile_offsets[], traffic_profile_offsets[],
    perpetrator_profile_offsets[];
Dword *construct_car_profile(Dword *, int);
Dword *construct_perpetrator_profile(Dword *, int);
Dword *construct_helicopter_profile(Dword *, Dword);
void construct_spike_profile(void *);
void *initialize_racer_traits(void *, Dword, Dword, Dword, Dword);
void *initialize_traffic_traits(void *, Dword);
Dword __fastcall load_profile_identifier(void *, void *, ini::Reader *, const char *);
Dword __fastcall load_car_profile(void *, void *, ini::Reader *, const char *);
Dword __fastcall load_racer_profile(void *, void *, ini::Reader *, const char *);
Dword __fastcall load_traffic_profile(void *, void *, ini::Reader *, const char *);
Dword __fastcall load_helicopter_profile(void *, void *, ini::Reader *, const char *);
int parse_entity_type(const char *);
Dword parse_profile_colour(const char *);
struct ProfileColour {
    Dword value, mask;
    const char *name;
    Dword argb;
};
extern const ProfileColour profile_colours[14];
} // namespace nfs::game::runtime::ai::lifecycle::pending
