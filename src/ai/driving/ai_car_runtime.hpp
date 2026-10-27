#pragma once
#include "ai_car_methods.hpp"
#include "nfs/network/session/network_world.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstring>
namespace nfs::application::pending {
void set_actor_finished(void *, std::uint8_t);
}
namespace nfs::game::runtime::ai::cars {
inline Dword word_pointer(const void *pointer) {
    return reinterpret_cast<Dword>(pointer);
}
inline float float_word(Dword bits) {
    float value;
    std::memcpy(&value, &bits, 4);
    return value;
}
inline void bind_tables(void *entity, const Dword *methods, const Dword *kind, const Dword *casts,
                        const Dword *body, const Dword *perpetrator = nullptr) {
    at<const Dword *>(entity, 0) = methods;
    at<const Dword *>(entities::interface_at(entity, 1), 0) = kind;
    at<const Dword *>(entities::interface_at(entity, 2), 0) = casts;
    if (body)
        at<const Dword *>(entities::interface_at(entity, 3), 0) = body;
    if (perpetrator)
        at<const Dword *>(entities::interface_at(entity, 4), 0) = perpetrator;
}
inline void set_displacement(void *entity, unsigned slot, int size) {
    const int displacement = at<const int *>(entity, 4)[slot];
    at<int>(entity, displacement) = displacement - size;
}
Dword *construct_car_entity(Dword *, void *, int);
Dword *construct_perpetrator(Dword *, int);
Dword *construct_cop_base(Dword *, void *, int);
Dword *construct_racer(Dword *, void *, int);
Dword *construct_pursuit_car(Dword *, void *, int);
Dword *construct_player(Dword *, void *, int);
Dword *construct_ai_cop(Dword *, void *, int);
Dword *construct_autonomous_cop(Dword *, void *, int);
Dword *construct_network_car(Dword *, void *, int);
void copy_racer_traits(void *, const void *);
void destroy_car_entity(void *);
void destroy_racer(void *);
void destroy_cop_base(void *);
void destroy_pursuit_car(void *);
void destroy_network_car(void *);
void destroy_traffic_car(void *);
void destroy_player(void *);
void destroy_ai_cop(void *);
void destroy_ai_racer(void *);
void destroy_autonomous_cop(void *);
void clear_tire_damage(void *);
void *chased_entity(void *);
void select_contact_voice(void *, const char *);
void apply_car_path_controls(void *, Dword);
extern const std::int32_t car_offsets[], perpetrator_offsets[], cop_base_offsets[],
    cop_base_perpetrator_offsets[], racer_offsets[], racer_perpetrator_offsets[], pursuit_offsets[],
    pursuit_perpetrator_offsets[], player_offsets[], player_perpetrator_offsets[], ai_cop_offsets[],
    ai_cop_perpetrator_offsets[], autonomous_cop_offsets[], autonomous_cop_perpetrator_offsets[],
    network_car_offsets[], network_car_perpetrator_offsets[], ai_racer_offsets[],
    ai_racer_perpetrator_offsets[], traffic_offsets[];
} // namespace nfs::game::runtime::ai::cars
