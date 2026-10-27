#include "ai_car_behavior.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/core/timing/frame_clock.hpp"
#include "nfs/core/timing/timer.hpp"
#include "nfs/network/session/network_runtime.hpp"
namespace nfs::application::pending {
void *race_participant_owner_view(void *, kernel::Dword);
}
namespace nfs::network {
kernel::Dword race_pending(kernel::Dword *);
}
namespace nfs::game::runtime::ai::cars {
using kernel::numeric::Extended;

Extended distance_to_local_players(const float *position) {
    return Extended(local_player_distance(position), kernel::numeric::current_format());
}
// Result is AL; callers consume the byte.
Dword __fastcall car_visible(void *entity, void *, Dword camera) {
    if (network::session_active() &&
        !should_draw_network_car(at<Dword>(at<void *>(entity, 368), 1772)))
        return 0;
    if (!at<Byte>(entity, 212) || frame_clock::paused ||
        !at<Dword>(at<void *>(at<void *>(entity, 368), 1768), 732) ||
        !camera_mode_flashes(reinterpret_cast<void *>(camera)))
        return 0;
    const std::int32_t time = kernel::timer_count();
    const std::int32_t frequency = kernel::timer_frequency;
    return time % (frequency / 6) > frequency / 12;
}
namespace {
void deactivate_finished_player(void *entity) {
    if (at<Dword>(system(72), 152) != 1 || network::session_active())
        return;
    void *participant = application::pending::race_participant_owner_view(
        system(92), at<Dword>(at<void *>(entity, 368), 1772));
    if (!participant || !(network::race_pending(static_cast<Dword *>(participant)) & 255))
        return;
    const float distance = local_player_distance(invoke<const float *>(entity, 0));
    if (!(distance > 300.f)) {
        void *body = invoke<void *>(entities::interface_at(entity, 3), 0);
        if (body && at<Byte>(body, 728))
            return;
    }
    lifecycle::deactivate_car_dynamics(static_cast<Dword *>(entity));
    invoke(entity, 6, Dword{0});
}
} // namespace

void __fastcall player_frame(void *entity, void *) {
    deactivate_finished_player(entity);
    car_frame(entity, nullptr);
}

void __fastcall player_recover_body(void *entity, void *, Dword controls) {
    deactivate_finished_player(entity);
    car_recover_body(entity, nullptr, controls);
}

Dword __fastcall player_recover_ground(void *entity, void *) {
    if (at<Dword>(system(72), 152) == 1) {
        void *participant = application::pending::race_participant_owner_view(
            system(92), at<Dword>(at<void *>(entity, 368), 1772));
        if (participant && (network::race_pending(static_cast<Dword *>(participant)) & 255)) {
            invoke(entity, 6, Dword{0});
            return 0;
        }
    }
    return car_recover_ground(entity, nullptr);
}

void __fastcall autonomous_cop_frame(void *entity, void *) {
    void *target = chased_entity(entity);
    bool reset = true;
    if (target) {
        void *own_section = lifecycle::path_current_section(bytes(entity) + 248);
        void *target_section = lifecycle::path_current_section(bytes(target) + 236);
        reset = target_section != own_section;
    }
    if (reset)
        clear_path_history(bytes(entity) + 220);
    car_frame(entity, nullptr);
}

} // namespace nfs::game::runtime::ai::cars
