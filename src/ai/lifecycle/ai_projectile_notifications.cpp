#include "../../police/pursuit/police_chase_x87.hpp"
#include "../entities/ai_entity_runtime.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include <cmath>
namespace nfs::game::runtime::ai::entities::pending {
Dword set_police_chase_outcome(void *, Dword);

void request_incident_camera(void *director, Dword delay, Dword event) {
    if (network::packets::rule_value(system(72), 42) && at<Dword>(system(72), 176) == 1 &&
        !network::session_active()) {
        void *behavior = at<void *>(game::world::find_camera_mode(director, 9), 80);
        if (!at<std::int16_t>(at<void *>(behavior, 32), 596)) {
            at<Dword>(director, 160) = event;
            at<Dword>(behavior, 16) = delay;
        }
    }
}

void notify_projectile_target(void *cop, Dword event) {
    if (at<Byte>(cop, 600))
        return;
    at<Byte>(cop, 600) = 1;
    for (unsigned player = 0; player < 2; ++player) {
        void *director = game::tick_services[player];
        if (!director)
            return;
        if (at<void *>(cop, 368) == at<void *>(director, 300)) {
            request_incident_camera(director, 0, event);
            return;
        }
    }
}

void apply_projectile_pursuit_damage(void *cop, float damage) {
    police_frame::apply_projectile_pursuit_damage_retained(cop, damage);
}
} // namespace nfs::game::runtime::ai::entities::pending
