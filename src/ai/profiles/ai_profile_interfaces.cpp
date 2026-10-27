#include "ai_profile_runtime.hpp"
#include "nfs/core/errors/assertions.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {
Dword __fastcall load_car_profile(void *profile, void *, ini::Reader *reader, const char *section);
Dword __fastcall cop_profile_payload(void *profile, void *);
Dword __fastcall load_cop_profile(void *profile, void *, ini::Reader *reader, const char *section);
Dword __fastcall profile_racer_view(void *profile, void *);
Dword __fastcall profile_player_view(void *profile, void *);
Dword __fastcall profile_player_cop_view(void *profile, void *);
Dword __fastcall profile_cop_view(void *profile, void *);
Dword __fastcall profile_perpetrator_view(void *profile, void *);
Dword __fastcall profile_net_view(void *profile, void *);
Dword __fastcall profile_helicopter_view(void *profile, void *);
Dword __fastcall profile_projectile_view(void *profile, void *);
Dword __fastcall profile_traffic_view(void *profile, void *);
Dword __fastcall profile_other_view(void *profile, void *);
Dword __fastcall profile_identifier(void *profile, void *);
Dword __fastcall profile_payload(void *profile, void *);
Dword __fastcall delete_base_profile(void *profile, void *, Dword flags);
Dword __fastcall projectile_profile_kind(void *profile, void *);
Dword __fastcall projectile_profile_payload(void *profile, void *);
Dword __fastcall load_projectile_profile(void *profile, void *, ini::Reader *reader,
                                        const char *section);
Dword __fastcall delete_projectile_profile(void *profile, void *, Dword flags);
Dword __fastcall helicopter_profile_payload(void *profile, void *);
Dword __fastcall helicopter_profile_kind(void *profile, void *);
Dword __fastcall load_helicopter_profile(void *profile, void *, ini::Reader *reader,
                                        const char *section);
Dword __fastcall delete_helicopter_profile(void *profile, void *, Dword flags);
Dword __fastcall car_profile_delete_entry(void *profile, void *, Dword flags);
Dword __fastcall car_profile_payload_entry(void *profile, void *);
Dword __fastcall car_profile_load_entry(void *profile, void *, ini::Reader *reader,
                                       const char *section);
Dword __fastcall racer_base_delete_entry(void *profile, void *, Dword flags);
Dword __fastcall delete_racer_base_profile(void *profile, void *, Dword flags);
Dword __fastcall racer_base_payload_entry(void *profile, void *);
Dword __fastcall racer_base_payload(void *profile, void *);
Dword __fastcall racer_base_load_entry(void *profile, void *, ini::Reader *reader,
                                      const char *section);
Dword __fastcall load_racer_base_profile(void *profile, void *, ini::Reader *reader,
                                        const char *section);
Dword __fastcall racer_profile_kind_entry(void *profile, void *);
Dword __fastcall racer_profile_kind(void *profile, void *);
Dword __fastcall racer_profile_delete_entry(void *profile, void *, Dword flags);
Dword __fastcall delete_racer_profile(void *profile, void *, Dword flags);
Dword __fastcall racer_profile_view_entry(void *profile, void *);
Dword __fastcall racer_profile_view(void *profile, void *);
Dword __fastcall racer_profile_payload_entry(void *profile, void *);
Dword __fastcall racer_profile_payload(void *profile, void *);
Dword __fastcall racer_profile_load_entry(void *profile, void *, ini::Reader *reader,
                                         const char *section);
Dword __fastcall perpetrator_profile_kind_entry(void *profile, void *);
Dword __fastcall perpetrator_profile_kind(void *profile, void *);
Dword __fastcall perpetrator_profile_delete_entry(void *profile, void *, Dword flags);
Dword __fastcall delete_perpetrator_profile(void *profile, void *, Dword flags);
Dword __fastcall perpetrator_profile_payload_entry(void *profile, void *);
Dword __fastcall perpetrator_profile_payload(void *profile, void *);
Dword __fastcall perpetrator_profile_view_entry(void *profile, void *);
Dword __fastcall perpetrator_profile_view(void *profile, void *);
Dword __fastcall car_profile_payload(void *profile, void *);
Dword __fastcall delete_car_profile(void *profile, void *, Dword flags);
Dword __fastcall load_profile_identifier(void *profile, void *, ini::Reader *reader,
                                        const char *section);
Dword __fastcall delete_spike_profile(void *profile, void *, Dword flags);
Dword __fastcall spike_profile_payload(void *profile, void *);
Dword __fastcall cop_profile_delete_entry(void *profile, void *, Dword flags);
Dword __fastcall delete_cop_profile(void *profile, void *, Dword flags);
Dword __fastcall cop_profile_payload_entry(void *profile, void *);
Dword __fastcall cop_profile_load_entry(void *profile, void *, ini::Reader *reader,
                                       const char *section);
Dword __fastcall player_profile_kind_entry(void *profile, void *);
Dword __fastcall player_profile_kind(void *profile, void *);
Dword __fastcall player_profile_delete_entry(void *profile, void *, Dword flags);
Dword __fastcall delete_player_profile(void *profile, void *, Dword flags);
Dword __fastcall player_profile_view_entry(void *profile, void *);
Dword __fastcall player_profile_view(void *profile, void *);
Dword __fastcall player_profile_payload_entry(void *profile, void *);
Dword __fastcall player_profile_payload(void *profile, void *);
Dword __fastcall player_profile_load_entry(void *profile, void *, ini::Reader *reader,
                                          const char *section);
Dword __fastcall load_player_profile(void *profile, void *, ini::Reader *reader,
                                    const char *section);
Dword __fastcall player_cop_profile_kind_entry(void *profile, void *);
Dword __fastcall player_cop_profile_kind(void *profile, void *);
Dword __fastcall player_cop_profile_delete_entry(void *profile, void *, Dword flags);
Dword __fastcall delete_player_cop_profile(void *profile, void *, Dword flags);
Dword __fastcall player_cop_profile_view_entry(void *profile, void *);
Dword __fastcall player_cop_profile_view(void *profile, void *);
Dword __fastcall ai_cop_profile_kind_entry(void *profile, void *);
Dword __fastcall ai_cop_profile_kind(void *profile, void *);
Dword __fastcall ai_cop_profile_delete_entry(void *profile, void *, Dword flags);
Dword __fastcall delete_ai_cop_profile(void *profile, void *, Dword flags);
Dword __fastcall ai_cop_profile_view_entry(void *profile, void *);
Dword __fastcall ai_cop_profile_view(void *profile, void *);
Dword __fastcall traffic_profile_kind_entry(void *profile, void *);
Dword __fastcall traffic_profile_delete_entry(void *profile, void *, Dword flags);
Dword __fastcall delete_traffic_profile(void *profile, void *, Dword flags);
Dword __fastcall traffic_profile_view_entry(void *profile, void *);
Dword __fastcall traffic_profile_view(void *profile, void *);
Dword __fastcall traffic_profile_payload_entry(void *profile, void *);
Dword __fastcall traffic_profile_payload(void *profile, void *);
Dword __fastcall traffic_profile_load_entry(void *profile, void *, ini::Reader *reader,
                                           const char *section);
Dword __fastcall load_racer_profile(void *profile, void *, ini::Reader *reader, const char *section);
Dword __fastcall spike_profile_kind(void *profile, void *);
Dword __fastcall load_spike_profile(void *profile, void *, ini::Reader *reader, const char *section);
Dword __fastcall net_profile_delete_entry(void *profile, void *, Dword flags);
Dword __fastcall delete_net_profile(void *profile, void *, Dword flags);
Dword __fastcall net_profile_view_entry(void *profile, void *);
Dword __fastcall net_profile_view(void *profile, void *);
Dword __fastcall net_profile_payload_entry(void *profile, void *);
Dword __fastcall net_profile_payload(void *profile, void *);
Dword __fastcall net_profile_load_entry(void *profile, void *, ini::Reader *reader,
                                       const char *section);
Dword __fastcall load_net_profile(void *profile, void *, ini::Reader *reader, const char *section);
Dword __fastcall traffic_profile_kind(void *profile, void *);
Dword __fastcall load_traffic_profile(void *profile, void *, ini::Reader *reader,
                                     const char *section);

void destroy_base_profile(void *profile) {
    at<const Dword *>(profile, 0) = profile_base_methods;
}
 
void destroy_racer_base_profile(void *profile) {
}
 
void destroy_racer_profile(void *profile) {
}
 
void destroy_perpetrator_profile(void *profile) {
}
 
void destroy_cop_profile(void *profile) {
}
 
void destroy_player_profile(void *profile) {
}
 
void destroy_player_cop_profile(void *profile) {
}
 
void destroy_ai_cop_profile(void *profile) {
}
 
void destroy_traffic_profile(void *profile) {
}
 
void destroy_net_profile(void *profile) {
}
 
void destroy_helicopter_profile(void *profile) {
    destroy_base_profile(profile);
}
 
void destroy_spike_profile(void *profile) {
    destroy_base_profile(profile);
}
 
void destroy_projectile_profile(void *profile) {
    destroy_base_profile(profile);
}

Dword __fastcall cop_profile_payload(void *profile, void *) {
    return 0;
}

Dword __fastcall load_cop_profile(void *profile, void *, ini::Reader *reader, const char *section) {
    return load_car_profile(profile, nullptr, reader, section);
}

Dword __fastcall profile_racer_view(void *profile, void *) {
    return 0;
}

Dword __fastcall profile_player_view(void *profile, void *) {
    return 0;
}

Dword __fastcall profile_player_cop_view(void *profile, void *) {
    return 0;
}

Dword __fastcall profile_cop_view(void *profile, void *) {
    return 0;
}

Dword __fastcall profile_perpetrator_view(void *profile, void *) {
    return 0;
}

Dword __fastcall profile_net_view(void *profile, void *) {
    return 0;
}

Dword __fastcall profile_helicopter_view(void *profile, void *) {
    return 0;
}

Dword __fastcall profile_projectile_view(void *profile, void *) {
    return 0;
}

Dword __fastcall profile_traffic_view(void *profile, void *) {
    return 0;
}

Dword __fastcall profile_other_view(void *profile, void *) {
    return 0;
}

Dword __fastcall profile_identifier(void *profile, void *) {
    return at<Dword>(profile, 4);
}

Dword __fastcall profile_payload(void *profile, void *) {
    return 0;
}

Dword __fastcall delete_base_profile(void *profile, void *, Dword flags) {
    void *allocation = bytes(profile) - 0;
    destroy_base_profile(profile);
    if (flags & 1)
        gui::application_release(allocation);
    return reinterpret_cast<Dword>(allocation);
}

Dword __fastcall projectile_profile_kind(void *profile, void *) {
    return 9;
}

Dword __fastcall projectile_profile_payload(void *profile, void *) {
    return 1;
}

Dword __fastcall load_projectile_profile(void *profile, void *, ini::Reader *reader,
                                        const char *section) {
    return 1;
}

Dword __fastcall delete_projectile_profile(void *profile, void *, Dword flags) {
    void *allocation = bytes(profile) - 0;
    destroy_projectile_profile(profile);
    at<const Dword *>(profile, 0) = profile_base_methods;
    if (flags & 1)
        gui::application_release(allocation);
    return reinterpret_cast<Dword>(allocation);
}

Dword __fastcall helicopter_profile_payload(void *profile, void *) {
    return 0;
}

Dword __fastcall helicopter_profile_kind(void *profile, void *) {
    return 7;
}

Dword __fastcall delete_helicopter_profile(void *profile, void *, Dword flags) {
    void *allocation = bytes(profile) - 0;
    destroy_helicopter_profile(profile);
    if (flags & 1)
        gui::application_release(allocation);
    return reinterpret_cast<Dword>(allocation);
}

Dword __fastcall car_profile_delete_entry(void *profile, void *, Dword flags) {
    return delete_car_profile(adjust_profile_interface(profile), nullptr, flags);
}

Dword __fastcall car_profile_payload_entry(void *profile, void *) {
    return car_profile_payload(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall car_profile_load_entry(void *profile, void *, ini::Reader *reader,
                                       const char *section) {
    return load_car_profile(adjust_profile_interface(profile), nullptr, reader, section);
}

Dword __fastcall racer_base_delete_entry(void *profile, void *, Dword flags) {
    return delete_racer_base_profile(adjust_profile_interface(profile), nullptr, flags);
}

Dword __fastcall delete_racer_base_profile(void *profile, void *, Dword flags) {
    void *allocation = bytes(profile) - 164;
    destroy_racer_base_profile(profile);
    at<const Dword *>(profile, 0) = profile_base_methods;
    if (flags & 1)
        gui::application_release(allocation);
    return reinterpret_cast<Dword>(allocation);
}

Dword __fastcall racer_base_payload_entry(void *profile, void *) {
    return racer_base_payload(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall racer_base_payload(void *profile, void *) {
    return 0;
}

Dword __fastcall racer_base_load_entry(void *profile, void *, ini::Reader *reader,
                                      const char *section) {
    return load_racer_base_profile(adjust_profile_interface(profile), nullptr, reader, section);
}

Dword __fastcall load_racer_base_profile(void *profile, void *, ini::Reader *reader,
                                        const char *section) {
    return load_car_profile(profile, nullptr, reader, section);
}

Dword __fastcall racer_profile_kind_entry(void *profile, void *) {
    return racer_profile_kind(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall racer_profile_kind(void *profile, void *) {
    return profile_field<Dword>(profile, -8);
}

Dword __fastcall racer_profile_delete_entry(void *profile, void *, Dword flags) {
    return delete_racer_profile(adjust_profile_interface(profile), nullptr, flags);
}

Dword __fastcall delete_racer_profile(void *profile, void *, Dword flags) {
    void *allocation = bytes(profile) - 184;
    destroy_racer_profile(profile);
    at<const Dword *>(profile, 0) = profile_base_methods;
    if (flags & 1)
        gui::application_release(allocation);
    return reinterpret_cast<Dword>(allocation);
}

Dword __fastcall racer_profile_view_entry(void *profile, void *) {
    return racer_profile_view(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall racer_profile_view(void *profile, void *) {
    return reinterpret_cast<Dword>(bytes(profile) - 184);
}

Dword __fastcall racer_profile_payload_entry(void *profile, void *) {
    return racer_profile_payload(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall racer_profile_payload(void *profile, void *) {
    return 0;
}

Dword __fastcall racer_profile_load_entry(void *profile, void *, ini::Reader *reader,
                                         const char *section) {
    return load_racer_profile(adjust_profile_interface(profile), nullptr, reader, section);
}

Dword __fastcall perpetrator_profile_kind_entry(void *profile, void *) {
    return perpetrator_profile_kind(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall perpetrator_profile_kind(void *profile, void *) {
    return 5;
}

Dword __fastcall perpetrator_profile_delete_entry(void *profile, void *, Dword flags) {
    return delete_perpetrator_profile(adjust_profile_interface(profile), nullptr, flags);
}

Dword __fastcall delete_perpetrator_profile(void *profile, void *, Dword flags) {
    void *allocation = bytes(profile) - 184;
    destroy_perpetrator_profile(profile);
    at<const Dword *>(profile, 0) = profile_base_methods;
    if (flags & 1)
        gui::application_release(allocation);
    return reinterpret_cast<Dword>(allocation);
}

Dword __fastcall perpetrator_profile_payload_entry(void *profile, void *) {
    return perpetrator_profile_payload(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall perpetrator_profile_payload(void *profile, void *) {
    return 0;
}

Dword __fastcall perpetrator_profile_view_entry(void *profile, void *) {
    return perpetrator_profile_view(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall perpetrator_profile_view(void *profile, void *) {
    return reinterpret_cast<Dword>(bytes(profile) - 184);
}

Dword __fastcall car_profile_payload(void *profile, void *) {
    return 0;
}

Dword __fastcall delete_car_profile(void *profile, void *, Dword flags) {
    void *allocation = bytes(profile) - 164;
    at<const Dword *>(profile, 0) = profile_base_methods;
    if (flags & 1)
        gui::application_release(allocation);
    return reinterpret_cast<Dword>(allocation);
}

Dword __fastcall delete_spike_profile(void *profile, void *, Dword flags) {
    void *allocation = bytes(profile) - 0;
    destroy_spike_profile(profile);
    if (flags & 1)
        gui::application_release(allocation);
    return reinterpret_cast<Dword>(allocation);
}

Dword __fastcall spike_profile_payload(void *profile, void *) {
    return 1;
}

Dword __fastcall cop_profile_delete_entry(void *profile, void *, Dword flags) {
    return delete_cop_profile(adjust_profile_interface(profile), nullptr, flags);
}

Dword __fastcall delete_cop_profile(void *profile, void *, Dword flags) {
    void *allocation = bytes(profile) - 164;
    destroy_cop_profile(profile);
    at<const Dword *>(profile, 0) = profile_base_methods;
    if (flags & 1)
        gui::application_release(allocation);
    return reinterpret_cast<Dword>(allocation);
}

Dword __fastcall cop_profile_payload_entry(void *profile, void *) {
    return cop_profile_payload(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall cop_profile_load_entry(void *profile, void *, ini::Reader *reader,
                                       const char *section) {
    return load_cop_profile(adjust_profile_interface(profile), nullptr, reader, section);
}

Dword __fastcall player_profile_kind_entry(void *profile, void *) {
    return player_profile_kind(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall player_profile_kind(void *profile, void *) {
    return 1;
}

Dword __fastcall player_profile_delete_entry(void *profile, void *, Dword flags) {
    return delete_player_profile(adjust_profile_interface(profile), nullptr, flags);
}

Dword __fastcall delete_player_profile(void *profile, void *, Dword flags) {
    void *allocation = bytes(profile) - 164;
    destroy_player_profile(profile);
    at<const Dword *>(profile, 0) = profile_base_methods;
    if (flags & 1)
        gui::application_release(allocation);
    return reinterpret_cast<Dword>(allocation);
}

Dword __fastcall player_profile_view_entry(void *profile, void *) {
    return player_profile_view(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall player_profile_view(void *profile, void *) {
    return reinterpret_cast<Dword>(bytes(profile) - 164);
}

Dword __fastcall player_profile_payload_entry(void *profile, void *) {
    return player_profile_payload(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall player_profile_payload(void *profile, void *) {
    return 0;
}

Dword __fastcall player_profile_load_entry(void *profile, void *, ini::Reader *reader,
                                          const char *section) {
    return load_player_profile(adjust_profile_interface(profile), nullptr, reader, section);
}

Dword __fastcall load_player_profile(void *profile, void *, ini::Reader *reader,
                                    const char *section) {
    return load_car_profile(profile, nullptr, reader, section);
}

Dword __fastcall player_cop_profile_kind_entry(void *profile, void *) {
    return player_cop_profile_kind(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall player_cop_profile_kind(void *profile, void *) {
    return 2;
}

Dword __fastcall player_cop_profile_delete_entry(void *profile, void *, Dword flags) {
    return delete_player_cop_profile(adjust_profile_interface(profile), nullptr, flags);
}

Dword __fastcall delete_player_cop_profile(void *profile, void *, Dword flags) {
    void *allocation = bytes(profile) - 164;
    destroy_player_cop_profile(profile);
    at<const Dword *>(profile, 0) = profile_base_methods;
    if (flags & 1)
        gui::application_release(allocation);
    return reinterpret_cast<Dword>(allocation);
}

Dword __fastcall player_cop_profile_view_entry(void *profile, void *) {
    return player_cop_profile_view(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall player_cop_profile_view(void *profile, void *) {
    return reinterpret_cast<Dword>(bytes(profile) - 164);
}

Dword __fastcall ai_cop_profile_kind_entry(void *profile, void *) {
    return ai_cop_profile_kind(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall ai_cop_profile_kind(void *profile, void *) {
    return 3;
}

Dword __fastcall ai_cop_profile_delete_entry(void *profile, void *, Dword flags) {
    return delete_ai_cop_profile(adjust_profile_interface(profile), nullptr, flags);
}

Dword __fastcall delete_ai_cop_profile(void *profile, void *, Dword flags) {
    void *allocation = bytes(profile) - 164;
    destroy_ai_cop_profile(profile);
    at<const Dword *>(profile, 0) = profile_base_methods;
    if (flags & 1)
        gui::application_release(allocation);
    return reinterpret_cast<Dword>(allocation);
}

Dword __fastcall ai_cop_profile_view_entry(void *profile, void *) {
    return ai_cop_profile_view(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall ai_cop_profile_view(void *profile, void *) {
    return reinterpret_cast<Dword>(bytes(profile) - 164);
}

Dword __fastcall traffic_profile_kind_entry(void *profile, void *) {
    return traffic_profile_kind(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall traffic_profile_delete_entry(void *profile, void *, Dword flags) {
    return delete_traffic_profile(adjust_profile_interface(profile), nullptr, flags);
}

Dword __fastcall delete_traffic_profile(void *profile, void *, Dword flags) {
    void *allocation = bytes(profile) - 172;
    destroy_traffic_profile(profile);
    at<const Dword *>(profile, 0) = profile_base_methods;
    if (flags & 1)
        gui::application_release(allocation);
    return reinterpret_cast<Dword>(allocation);
}

Dword __fastcall traffic_profile_view_entry(void *profile, void *) {
    return traffic_profile_view(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall traffic_profile_view(void *profile, void *) {
    return reinterpret_cast<Dword>(bytes(profile) - 172);
}

Dword __fastcall traffic_profile_payload_entry(void *profile, void *) {
    return traffic_profile_payload(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall traffic_profile_payload(void *profile, void *) {
    return 0;
}

Dword __fastcall traffic_profile_load_entry(void *profile, void *, ini::Reader *reader,
                                           const char *section) {
    return load_traffic_profile(adjust_profile_interface(profile), nullptr, reader, section);
}

Dword __fastcall spike_profile_kind(void *profile, void *) {
    return 8;
}

Dword __fastcall load_spike_profile(void *profile, void *, ini::Reader *reader,
                                   const char *section) {
    return 1;
}

Dword __fastcall net_profile_delete_entry(void *profile, void *, Dword flags) {
    return delete_net_profile(adjust_profile_interface(profile), nullptr, flags);
}

Dword __fastcall delete_net_profile(void *profile, void *, Dword flags) {
    void *allocation = bytes(profile) - 184;
    destroy_net_profile(profile);
    at<const Dword *>(profile, 0) = profile_base_methods;
    if (flags & 1)
        gui::application_release(allocation);
    return reinterpret_cast<Dword>(allocation);
}

Dword __fastcall net_profile_view_entry(void *profile, void *) {
    return net_profile_view(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall net_profile_view(void *profile, void *) {
    return reinterpret_cast<Dword>(bytes(profile) - 184);
}

Dword __fastcall net_profile_payload_entry(void *profile, void *) {
    return net_profile_payload(adjust_profile_interface(profile), nullptr);
}

Dword __fastcall net_profile_payload(void *profile, void *) {
    return 0;
}

Dword __fastcall net_profile_load_entry(void *profile, void *, ini::Reader *reader,
                                       const char *section) {
    return load_net_profile(adjust_profile_interface(profile), nullptr, reader, section);
}

Dword __fastcall load_net_profile(void *profile, void *, ini::Reader *reader, const char *section) {
    return load_racer_profile(profile, nullptr, reader, section);
}

Dword __fastcall traffic_profile_kind(void *profile, void *) {
    return 4;
}

const Dword profile_base_methods[] = {reinterpret_cast<Dword>(assertions::pure_virtual_call),
                                     reinterpret_cast<Dword>(delete_base_profile),
                                     reinterpret_cast<Dword>(profile_racer_view),
                                     reinterpret_cast<Dword>(profile_player_view),
                                     reinterpret_cast<Dword>(profile_player_cop_view),
                                     reinterpret_cast<Dword>(profile_cop_view),
                                     reinterpret_cast<Dword>(profile_perpetrator_view),
                                     reinterpret_cast<Dword>(profile_net_view),
                                     reinterpret_cast<Dword>(profile_helicopter_view),
                                     reinterpret_cast<Dword>(profile_projectile_view),
                                     reinterpret_cast<Dword>(profile_traffic_view),
                                     reinterpret_cast<Dword>(profile_other_view),
                                     reinterpret_cast<Dword>(profile_identifier),
                                     reinterpret_cast<Dword>(profile_payload),
                                     reinterpret_cast<Dword>(load_profile_identifier)};

const Dword racer_profile_methods[] = {reinterpret_cast<Dword>(racer_profile_kind_entry),
                                      reinterpret_cast<Dword>(racer_profile_delete_entry),
                                      reinterpret_cast<Dword>(racer_profile_view_entry),
                                      reinterpret_cast<Dword>(profile_player_view),
                                      reinterpret_cast<Dword>(profile_player_cop_view),
                                      reinterpret_cast<Dword>(profile_cop_view),
                                      reinterpret_cast<Dword>(profile_perpetrator_view),
                                      reinterpret_cast<Dword>(profile_net_view),
                                      reinterpret_cast<Dword>(profile_helicopter_view),
                                      reinterpret_cast<Dword>(profile_projectile_view),
                                      reinterpret_cast<Dword>(profile_traffic_view),
                                      reinterpret_cast<Dword>(profile_other_view),
                                      reinterpret_cast<Dword>(profile_identifier),
                                      reinterpret_cast<Dword>(racer_profile_payload_entry),
                                      reinterpret_cast<Dword>(racer_profile_load_entry)};

const Dword racer_base_methods[] = {reinterpret_cast<Dword>(assertions::pure_virtual_call),
                                   reinterpret_cast<Dword>(racer_base_delete_entry),
                                   reinterpret_cast<Dword>(profile_racer_view),
                                   reinterpret_cast<Dword>(profile_player_view),
                                   reinterpret_cast<Dword>(profile_player_cop_view),
                                   reinterpret_cast<Dword>(profile_cop_view),
                                   reinterpret_cast<Dword>(profile_perpetrator_view),
                                   reinterpret_cast<Dword>(profile_net_view),
                                   reinterpret_cast<Dword>(profile_helicopter_view),
                                   reinterpret_cast<Dword>(profile_projectile_view),
                                   reinterpret_cast<Dword>(profile_traffic_view),
                                   reinterpret_cast<Dword>(profile_other_view),
                                   reinterpret_cast<Dword>(profile_identifier),
                                   reinterpret_cast<Dword>(racer_base_payload_entry),
                                   reinterpret_cast<Dword>(racer_base_load_entry)};

const Dword car_profile_methods[] = {reinterpret_cast<Dword>(assertions::pure_virtual_call),
                                    reinterpret_cast<Dword>(car_profile_delete_entry),
                                    reinterpret_cast<Dword>(profile_racer_view),
                                    reinterpret_cast<Dword>(profile_player_view),
                                    reinterpret_cast<Dword>(profile_player_cop_view),
                                    reinterpret_cast<Dword>(profile_cop_view),
                                    reinterpret_cast<Dword>(profile_perpetrator_view),
                                    reinterpret_cast<Dword>(profile_net_view),
                                    reinterpret_cast<Dword>(profile_helicopter_view),
                                    reinterpret_cast<Dword>(profile_projectile_view),
                                    reinterpret_cast<Dword>(profile_traffic_view),
                                    reinterpret_cast<Dword>(profile_other_view),
                                    reinterpret_cast<Dword>(profile_identifier),
                                    reinterpret_cast<Dword>(car_profile_payload_entry),
                                    reinterpret_cast<Dword>(car_profile_load_entry)};

Dword __fastcall spike_profile_self(void *profile, void *) {
    return reinterpret_cast<Dword>(profile);
}

Dword __fastcall helicopter_profile_self(void *profile, void *) {
    return reinterpret_cast<Dword>(profile);
}

Dword __fastcall projectile_profile_self(void *profile, void *) {
    return reinterpret_cast<Dword>(profile);
}

const Dword spike_profile_methods[] = {reinterpret_cast<Dword>(spike_profile_kind),
                                      reinterpret_cast<Dword>(delete_spike_profile),
                                      reinterpret_cast<Dword>(profile_racer_view),
                                      reinterpret_cast<Dword>(profile_player_view),
                                      reinterpret_cast<Dword>(profile_player_cop_view),
                                      reinterpret_cast<Dword>(profile_cop_view),
                                      reinterpret_cast<Dword>(profile_perpetrator_view),
                                      reinterpret_cast<Dword>(profile_net_view),
                                      reinterpret_cast<Dword>(profile_helicopter_view),
                                      reinterpret_cast<Dword>(profile_projectile_view),
                                      reinterpret_cast<Dword>(profile_traffic_view),
                                      reinterpret_cast<Dword>(profile_other_view),
                                      reinterpret_cast<Dword>(profile_identifier),
                                      reinterpret_cast<Dword>(spike_profile_payload),
                                       reinterpret_cast<Dword>(load_spike_profile),
                                       reinterpret_cast<Dword>(spike_profile_self)};

const Dword traffic_profile_methods[] = {reinterpret_cast<Dword>(traffic_profile_kind_entry),
                                        reinterpret_cast<Dword>(traffic_profile_delete_entry),
                                        reinterpret_cast<Dword>(profile_racer_view),
                                        reinterpret_cast<Dword>(profile_player_view),
                                        reinterpret_cast<Dword>(profile_player_cop_view),
                                        reinterpret_cast<Dword>(profile_cop_view),
                                        reinterpret_cast<Dword>(profile_perpetrator_view),
                                        reinterpret_cast<Dword>(profile_net_view),
                                        reinterpret_cast<Dword>(profile_helicopter_view),
                                        reinterpret_cast<Dword>(profile_projectile_view),
                                        reinterpret_cast<Dword>(traffic_profile_view_entry),
                                        reinterpret_cast<Dword>(profile_other_view),
                                        reinterpret_cast<Dword>(profile_identifier),
                                        reinterpret_cast<Dword>(traffic_profile_payload_entry),
                                        reinterpret_cast<Dword>(traffic_profile_load_entry)};

const Dword ai_cop_profile_methods[] = {reinterpret_cast<Dword>(ai_cop_profile_kind_entry),
                                       reinterpret_cast<Dword>(ai_cop_profile_delete_entry),
                                       reinterpret_cast<Dword>(profile_racer_view),
                                       reinterpret_cast<Dword>(profile_player_view),
                                       reinterpret_cast<Dword>(profile_player_cop_view),
                                       reinterpret_cast<Dword>(ai_cop_profile_view_entry),
                                       reinterpret_cast<Dword>(profile_perpetrator_view),
                                       reinterpret_cast<Dword>(profile_net_view),
                                       reinterpret_cast<Dword>(profile_helicopter_view),
                                       reinterpret_cast<Dword>(profile_projectile_view),
                                       reinterpret_cast<Dword>(profile_traffic_view),
                                       reinterpret_cast<Dword>(profile_other_view),
                                       reinterpret_cast<Dword>(profile_identifier),
                                       reinterpret_cast<Dword>(cop_profile_payload_entry),
                                       reinterpret_cast<Dword>(cop_profile_load_entry)};

const Dword player_cop_profile_methods[] = {reinterpret_cast<Dword>(player_cop_profile_kind_entry),
                                           reinterpret_cast<Dword>(player_cop_profile_delete_entry),
                                           reinterpret_cast<Dword>(profile_racer_view),
                                           reinterpret_cast<Dword>(profile_player_view),
                                           reinterpret_cast<Dword>(player_cop_profile_view_entry),
                                           reinterpret_cast<Dword>(profile_cop_view),
                                           reinterpret_cast<Dword>(profile_perpetrator_view),
                                           reinterpret_cast<Dword>(profile_net_view),
                                           reinterpret_cast<Dword>(profile_helicopter_view),
                                           reinterpret_cast<Dword>(profile_projectile_view),
                                           reinterpret_cast<Dword>(profile_traffic_view),
                                           reinterpret_cast<Dword>(profile_other_view),
                                           reinterpret_cast<Dword>(profile_identifier),
                                           reinterpret_cast<Dword>(cop_profile_payload_entry),
                                           reinterpret_cast<Dword>(cop_profile_load_entry)};

const Dword cop_profile_methods[] = {reinterpret_cast<Dword>(assertions::pure_virtual_call),
                                    reinterpret_cast<Dword>(cop_profile_delete_entry),
                                    reinterpret_cast<Dword>(profile_racer_view),
                                    reinterpret_cast<Dword>(profile_player_view),
                                    reinterpret_cast<Dword>(profile_player_cop_view),
                                    reinterpret_cast<Dword>(profile_cop_view),
                                    reinterpret_cast<Dword>(profile_perpetrator_view),
                                    reinterpret_cast<Dword>(profile_net_view),
                                    reinterpret_cast<Dword>(profile_helicopter_view),
                                    reinterpret_cast<Dword>(profile_projectile_view),
                                    reinterpret_cast<Dword>(profile_traffic_view),
                                    reinterpret_cast<Dword>(profile_other_view),
                                    reinterpret_cast<Dword>(profile_identifier),
                                    reinterpret_cast<Dword>(cop_profile_payload_entry),
                                    reinterpret_cast<Dword>(cop_profile_load_entry)};

const Dword player_profile_methods[] = {reinterpret_cast<Dword>(player_profile_kind_entry),
                                       reinterpret_cast<Dword>(player_profile_delete_entry),
                                       reinterpret_cast<Dword>(profile_racer_view),
                                       reinterpret_cast<Dword>(player_profile_view_entry),
                                       reinterpret_cast<Dword>(profile_player_cop_view),
                                       reinterpret_cast<Dword>(profile_cop_view),
                                       reinterpret_cast<Dword>(profile_perpetrator_view),
                                       reinterpret_cast<Dword>(profile_net_view),
                                       reinterpret_cast<Dword>(profile_helicopter_view),
                                       reinterpret_cast<Dword>(profile_projectile_view),
                                       reinterpret_cast<Dword>(profile_traffic_view),
                                       reinterpret_cast<Dword>(profile_other_view),
                                       reinterpret_cast<Dword>(profile_identifier),
                                       reinterpret_cast<Dword>(player_profile_payload_entry),
                                       reinterpret_cast<Dword>(player_profile_load_entry)};

const Dword net_profile_methods[] = {reinterpret_cast<Dword>(racer_profile_kind_entry),
                                    reinterpret_cast<Dword>(net_profile_delete_entry),
                                    reinterpret_cast<Dword>(racer_profile_view_entry),
                                    reinterpret_cast<Dword>(profile_player_view),
                                    reinterpret_cast<Dword>(profile_player_cop_view),
                                    reinterpret_cast<Dword>(profile_cop_view),
                                    reinterpret_cast<Dword>(profile_perpetrator_view),
                                    reinterpret_cast<Dword>(net_profile_view_entry),
                                    reinterpret_cast<Dword>(profile_helicopter_view),
                                    reinterpret_cast<Dword>(profile_projectile_view),
                                    reinterpret_cast<Dword>(profile_traffic_view),
                                    reinterpret_cast<Dword>(profile_other_view),
                                    reinterpret_cast<Dword>(profile_identifier),
                                    reinterpret_cast<Dword>(net_profile_payload_entry),
                                    reinterpret_cast<Dword>(net_profile_load_entry)};

const Dword perpetrator_profile_methods[] = {
    reinterpret_cast<Dword>(perpetrator_profile_kind_entry),
    reinterpret_cast<Dword>(perpetrator_profile_delete_entry),
    reinterpret_cast<Dword>(perpetrator_profile_payload_entry),
    reinterpret_cast<Dword>(profile_player_view),
    reinterpret_cast<Dword>(profile_player_cop_view),
    reinterpret_cast<Dword>(profile_cop_view),
    reinterpret_cast<Dword>(perpetrator_profile_view_entry),
    reinterpret_cast<Dword>(profile_net_view),
    reinterpret_cast<Dword>(profile_helicopter_view),
    reinterpret_cast<Dword>(profile_projectile_view),
    reinterpret_cast<Dword>(profile_traffic_view),
    reinterpret_cast<Dword>(profile_other_view),
    reinterpret_cast<Dword>(profile_identifier),
    reinterpret_cast<Dword>(racer_profile_payload_entry),
    reinterpret_cast<Dword>(racer_profile_load_entry)};

const Dword helicopter_profile_methods[] = {reinterpret_cast<Dword>(helicopter_profile_kind),
                                           reinterpret_cast<Dword>(delete_helicopter_profile),
                                           reinterpret_cast<Dword>(profile_racer_view),
                                           reinterpret_cast<Dword>(profile_player_view),
                                           reinterpret_cast<Dword>(profile_player_cop_view),
                                           reinterpret_cast<Dword>(profile_cop_view),
                                           reinterpret_cast<Dword>(profile_perpetrator_view),
                                           reinterpret_cast<Dword>(profile_net_view),
                                           reinterpret_cast<Dword>(profile_helicopter_view),
                                           reinterpret_cast<Dword>(profile_projectile_view),
                                           reinterpret_cast<Dword>(profile_traffic_view),
                                           reinterpret_cast<Dword>(profile_other_view),
                                           reinterpret_cast<Dword>(profile_identifier),
                                           reinterpret_cast<Dword>(helicopter_profile_payload),
                                           reinterpret_cast<Dword>(load_helicopter_profile),
                                           reinterpret_cast<Dword>(helicopter_profile_self)};

const Dword projectile_profile_methods[] = {reinterpret_cast<Dword>(projectile_profile_kind),
                                           reinterpret_cast<Dword>(delete_projectile_profile),
                                           reinterpret_cast<Dword>(profile_racer_view),
                                           reinterpret_cast<Dword>(profile_player_view),
                                           reinterpret_cast<Dword>(profile_player_cop_view),
                                           reinterpret_cast<Dword>(profile_cop_view),
                                           reinterpret_cast<Dword>(profile_perpetrator_view),
                                           reinterpret_cast<Dword>(profile_net_view),
                                           reinterpret_cast<Dword>(profile_helicopter_view),
                                           reinterpret_cast<Dword>(profile_projectile_view),
                                           reinterpret_cast<Dword>(profile_traffic_view),
                                           reinterpret_cast<Dword>(profile_other_view),
                                           reinterpret_cast<Dword>(profile_identifier),
                                           reinterpret_cast<Dword>(projectile_profile_payload),
                                           reinterpret_cast<Dword>(load_projectile_profile),
                                           reinterpret_cast<Dword>(projectile_profile_self)};
} // namespace nfs::game::runtime::ai::lifecycle::pending
