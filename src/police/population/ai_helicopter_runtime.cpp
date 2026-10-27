#include "../pursuit/police_chase_x87.hpp"
#include "../../ai/entities/ai_entity_runtime.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include <cmath>
namespace nfs::game::runtime::ai::entities {
namespace pending {
void select_helicopter_destination(void *);
void fire_helicopter_projectiles(void *);
void steer_helicopter_pursuit(void *);
void steer_helicopter_patrol(void *);
void apply_helicopter_controls(void *, float, const float *);
void emit_helicopter_dust(void *);
Byte target_route_branch(void *, int *, float *, Dword *);
Byte update_path_sample(void *, const float *);
} // namespace pending
extern float helicopter_fire_distance;

void __fastcall advance_helicopter(void *entity, void *) {
    if (!at<Byte>(entity, 212))
        return;
    void *target = at<void *>(entity, 408);
    if (target) {
        void *path = bytes(entity) + 220;
        at<Dword>(path, 32) = at<Dword>(path, 28) = at<Dword>(path, 36) = 0;
        if (!at<Byte>(entity, 1660)) {
            if (void *vehicle = at<void *>(target, 44)) {
                const long double distance = ai::pair_distance(entity, vehicle);
                at<Byte>(entity, 1660) =
                    distance < helicopter_fire_distance || std::isnan(distance);
                invoke(entity, 31);
                pending::select_helicopter_destination(entity);
                return;
            }
        } else
            pending::fire_helicopter_projectiles(entity);
    }
    invoke(entity, 31);
    pending::select_helicopter_destination(entity);
}

void update_entity_path_sample(void *entity) {
    const float *position = invoke<const float *>(entity, 0);
    pending::update_path_sample(bytes(entity) + 220, position);
}
// One ignored stack word is present in the RET4 ABI.
void __fastcall steer_helicopter(void *entity, void *, Dword) {
    update_entity_path_sample(entity);
    at<Dword>(entity, 416) = 0;
    at<Dword>(entity, 420) = 0;
    const int mode = at<int>(entity, 380);
    if (mode != 1 && mode != 2)
        at<Dword>(entity, 332) = 0;
    if (mode == 1)
        pending::steer_helicopter_pursuit(entity);
    else if (mode == 2)
        pending::steer_helicopter_patrol(entity);
    const float *velocity = invoke<const float *>(entity, 29);
    pending::apply_helicopter_controls(entity, at<float>(entity, 396), velocity);
    pending::emit_helicopter_dust(entity);
}

void __fastcall set_helicopter_speed(void *entity, void *, float desired, Dword control_address) {
    police_frame::set_helicopter_speed_retained(entity, nullptr, desired, control_address);
}

Dword __fastcall choose_helicopter_route(void *entity, void *, void *branches) {
    const Dword count = at<std::uint16_t>(branches, 32);
    if (!count)
        return 0;
    if (count == 1)
        return at<Dword>(branches, 0);
    int direction;
    float distance;
    Dword branch;
    if (at<int>(entity, 380) == 1 &&
        pending::target_route_branch(entity, &direction, &distance, &branch)) {
        for (Dword index = 0; index < count; ++index)
            if (at<Dword>(branches, (index & 65535) * 4) == branch)
                return branch;
    }
    return choose_path_branch(entity, nullptr, branches);
}
} // namespace nfs::game::runtime::ai::entities
