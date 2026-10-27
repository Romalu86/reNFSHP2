#include "../driving/ai_drive_x87.hpp"
#include "player_drive_runtime.hpp"
namespace nfs::game::runtime::ai::cars {

void drive_player_before_start(void *entity, Dword controls) {
    drive_frame::player_before_start(entity, controls);
}

void drive_player_start_path(void *entity, Dword controls) {
    drive_frame::player_start_path(entity, controls);
}

void drive_player_path(void *entity, Dword controls) {
    drive_frame::player_path(entity, controls);
}
} // namespace nfs::game::runtime::ai::cars
