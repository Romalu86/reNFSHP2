#include "../../ai/driving/ai_drive_x87.hpp"
#include "network_pursuit_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "../../math/vectors/retained_vector.hpp"
namespace nfs::game::runtime::ai::cars {
using namespace lifecycle::pending;

void __fastcall network_car_drive(void *entity, void *, Dword controls_word) {
    auto *controls = reinterpret_cast<Dword *>(controls_word);
    Dword listener = 0;
    if (void *car = at<void *>(entity, 368))
        listener = at<Dword>(car, 1776);
    apply_car_path_controls(entity, controls_word);
    const Dword flags = controls[0];
    if (!(flags & (1u << 9)))
        at<Byte>(entity, 580) = 0;
    else {
        if (!at<Byte>(entity, 580))
            toggle_network_pursuit(entity);
        at<Byte>(entity, 580) = 1;
    }
    if (!(flags & (1u << 11)))
        at<Byte>(entity, 582) = 0;
    else {
        if (!network_chase(entity) || lifecycle::state<Dword>(7353308) ||
            int(now() - lifecycle::state<Dword>(7352944)) <= at<int>(entity, 576) ||
            at<Byte>(entity, 574) || at<Byte>(entity, 582))
            play_unavailable_action(listener);
        else if (request_network_spikes(entity))
            lifecycle::state<void *>(7353308) = network_chase(entity);
        at<Byte>(entity, 582) = 1;
    }
    if (!(flags & (1u << 10)))
        at<Byte>(entity, 581) = 0;
    else {
        if (!network_chase(entity) ||
            int(now() - lifecycle::state<Dword>(7352944)) <= at<int>(entity, 576) ||
            at<Byte>(entity, 573) || at<Byte>(entity, 581))
            play_unavailable_action(listener);
        else
            request_network_backup(entity);
        at<Byte>(entity, 581) = 1;
    }
    if (!(flags & (1u << 12)))
        at<Byte>(entity, 583) = 0;
    else {
        if (!network_chase(entity) ||
            int(now() - lifecycle::state<Dword>(7352944)) <= at<int>(entity, 576) ||
            at<Byte>(entity, 572) || at<Byte>(entity, 583))
            play_unavailable_action(listener);
        else
            request_network_helicopter(entity);
        at<Byte>(entity, 583) = 1;
    }
    if (at<Byte>(entity, 409)) {
        if (int(game::race_ticks) < 100)
            application::pending::set_actor_finished(entity, 0);
        else {
            const int duration = at<int>(entity, 416);
            const int elapsed = int(now() - at<Dword>(entity, 412));
            at<Byte>(controls, 5) = 0;
            at<Byte>(controls, 6) = 0;
            controls[0] = duration < elapsed ? 0 : (flags & 32768) | 2;
        }
    }
    if (at<Dword>(entity, 568) && at<int>(entity, 568) <= int(now())) {
        void *chase = network_chase(entity);
        at<Dword>(chase, 16) = at<Dword>(entity, 588);
        request_roadblock(&lifecycle::state<Dword>(7353160), chase);
        at<Dword>(entity, 568) = 0;
    }
}

void toggle_network_pursuit(void *entity) {
    drive_frame::toggle_pursuit(entity);
}
} // namespace nfs::game::runtime::ai::cars
