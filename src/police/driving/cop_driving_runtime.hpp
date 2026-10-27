#pragma once
#include "../../vehicles/frame/car_frame_runtime.hpp"
#include "../../network/session/network_pursuit_runtime.hpp"
#include "../../ai/corridor/car_corridor_runtime.hpp"
#include "../../ai/player/player_drive_runtime.hpp"
namespace nfs::game::world {
Dword find_supported_spawn(void *, float *);
}
namespace nfs::game::runtime::ai::cars {
Byte relocate_distant_chase_cop(void *);
Byte steer_cop_pursuit(void *, Dword *, float);
kernel::numeric::Extended car_route_heading(void *);
} // namespace nfs::game::runtime::ai::cars
