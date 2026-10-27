#pragma once
#include "ai_car_runtime.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include "nfs/network/session/network_player_state.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "../../vehicles/setup/car_initialization_runtime.hpp"
namespace nfs::game::runtime::physics {
void refresh_car_track_point(void *);
}
namespace nfs::game::runtime::ai::cars {
Dword body_palette_kind(void *, Dword);
Dword renderable_palette_kind(void *, Dword);
Dword choose_body_palette(void *, Dword);
void configure_racer_spawn(Dword *, Dword *, Dword, Dword, Dword);
void configure_cop_spawn(Dword *, Dword *, int, Dword, float *);
void register_spawned_participant(void *, Dword, Dword, const char *, const char *, Dword, Dword);
inline void append_race_entity(Dword *owner, Dword *entity) {
    if (at<int>(owner, 84172) < 8) {
        at<Dword *>(owner, 84176 + at<Dword>(owner, 84172) * 4) = entity;
        ++at<Dword>(owner, 84172);
    }
}
inline Dword *next_profile(Dword &index, Dword kind) {
    for (; index < lifecycle::state<Dword>(7353148); ++index) {
        Dword *profile = lifecycle::state<Dword *>(7352968 + index * 4);
        if (!profile || invoke<Dword>(profile, 0) == kind)
            return profile;
    }
    return nullptr;
}
inline void register_spawn(Dword *entity, Dword participant) {
    void *race = at<void *>(system(92), 4);
    const char *model = dependencies::car_model_name(entity[89]);
    const char *name = reinterpret_cast<const char *>(entity + 80);
    const Dword kind = invoke<Dword>(entities::interface_at(entity, 1), 0);
    register_spawned_participant(race, participant, kind, name, model, 0, 0);
}
} // namespace nfs::game::runtime::ai::cars
