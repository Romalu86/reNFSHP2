#include "../player/player_drive_runtime.hpp"
#include "ai_driver_runtime.hpp"
#include "ai_drive_x87.hpp"
#include "../../graphics/cameras/camera_route_math.hpp"
#include "../corridor/car_corridor_runtime.hpp"
#include <cstdint>
namespace nfs::game::runtime::ai::cars::drive_frame::legacy {
float compatibility_lane_floor(float shifted) {
    return float(std::floor(double(shifted)));
}
bool route_heading_below_half(void *entity) {
    return world::camera::route_math::heading_below_half(entity);
}
bool lateral_beyond_three_quarters(void *entity) {
    return world::camera::route_math::lateral_beyond_three_quarters(entity);
}
}

namespace nfs::game::runtime::ai::cars::drive_frame::legacy {
bool route_heading_negative(void *entity) {
    return world::camera::route_math::heading_negative(entity);
}
bool lateral_negative_for_target(void *entity, void *target) {
    return world::camera::route_math::lateral_negative_for_target(entity, target);
}
float lateral_f32(void *entity) {
    return world::camera::route_math::lateral_f32(entity);
}

}

namespace nfs::game::runtime::ai::cars {
kernel::numeric::Extended player_path_target_speed(void *entity, float requested) {
    alignas(4) unsigned char retained_speed[10];
    drive_frame::player_target_speed_native80(entity, requested, retained_speed);
    return kernel::numeric::Extended::from_native80(
        retained_speed, kernel::numeric::current_format());
}
} // namespace nfs::game::runtime::ai::cars
