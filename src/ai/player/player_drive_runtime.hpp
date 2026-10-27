#pragma once
#include "../driving/ai_driver_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include <cmath>
namespace nfs::game::runtime::ai::entities::pending {
extern float minimum_path_width;
}
namespace nfs::game::runtime::ai::cars {
Byte player_needs_reverse(void *);
void update_player_drive_state(void *);
void drive_player_before_start(void *, Dword);
void drive_player_path(void *, Dword);
void reverse_player_on_path(void *, Dword);
void recover_player_on_path(void *, Dword);
void drive_player_start_path(void *, Dword);
kernel::numeric::Extended player_path_target_speed(void *, float);
void drive_player_scripted_path(void *, Dword);
void drive_player_cinematic_path(void *, Dword);
void limit_player_turn_speed(void *, Dword, const float *, float);
} // namespace nfs::game::runtime::ai::cars
