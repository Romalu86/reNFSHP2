#pragma once
#include "ai_car_behavior.hpp"
namespace nfs::game::runtime::ai::entities {
Byte path_has_section(void *);
}
namespace nfs::game::runtime::ai::cars {
Byte traffic_yields_to_chase(void *);
Byte traffic_vehicle_ahead(void *);
Byte plan_cop_pursuit_route(void *, int *, float *, Dword *);
void update_cop_chase_controls(void *, Dword);
Byte detect_speeding_racers(void *);
} // namespace nfs::game::runtime::ai::cars
