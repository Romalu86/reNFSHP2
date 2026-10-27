#include "../driving/ai_drive_x87.hpp"
#include "../driving/ai_car_behavior.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
namespace nfs::application::pending {
void *traffic_actor_view(void *);
}
namespace nfs::game::runtime::ai::entities::pending {
void notify_projectile_target(void *, Dword);
}
namespace nfs::game::runtime::ai::cars {
// Callers ignore the return value.
void __fastcall ai_racer_drive(void *entity, void *, Dword controls_word) {
    void *car = at<void *>(entity, 368);
    if (apply_spike_strip_hits(entity) && !at<Byte>(entity, 600))
        entities::pending::notify_projectile_target(entity, 4);
    void *controls = reinterpret_cast<void *>(controls_word);
    apply_car_path_controls(entity, controls_word);
    const Dword hit_tick = at<Dword>(entity, 592);
    if (hit_tick && !at<Byte>(system(84), 88))
        at<Byte>(system(84), 88) = 1;
    const Dword now = graphics::unresolved::camera_state;
    if (hit_tick && std::int32_t(now - hit_tick) < 60) {
        at<Byte>(controls, 4) = at<Dword>(entity, 588) ? 127 : 129;
        at<Dword>(controls, 0) |= 2;
    }
    if (at<Byte>(car, 7952) && std::int32_t(now) >= std::int32_t(at<Dword>(car, 7956) + 600))
        at<Dword>(controls, 0) |= 2;
    if (at<Byte>(entity, 409)) {
        if (std::int32_t(game::race_ticks) < 100) {
            application::pending::set_actor_finished(entity, 0);
            return;
        }
        const Dword elapsed = now - at<Dword>(entity, 412);
        const Dword duration = at<Dword>(entity, 416);
        const Dword buttons = at<Dword>(controls, 0);
        at<Byte>(controls, 5) = 0;
        at<Byte>(controls, 6) = 0;
        at<Dword>(controls, 0) =
            (buttons & 33280) | (std::int32_t(elapsed) > std::int32_t(duration) ? 0 : 2);
    }
}

void __fastcall ai_racer_body_contact(void *entity, void *, Dword contact_word) {
    drive_frame::racer_body_contact(entity, contact_word);
}
// Only AL is defined on every return path.
Dword __fastcall player_accept_side_contact(void *entity, void *, Dword other_word, Dword speed_word) {
    return drive_frame::accept_player_side_contact(entity, other_word, speed_word);
}
} // namespace nfs::game::runtime::ai::cars
