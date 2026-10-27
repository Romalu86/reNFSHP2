#include "police_chase_x87.hpp"
#include "../../ai/driving/ai_car_behavior.hpp"
#include "../driving/cop_driving_runtime.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../graphics/cameras/camera_route_math.hpp"
namespace nfs::game::runtime::ai::police_math {
std::uint32_t truncate_native80(const void *value) {
    return std::uint32_t(hardware::truncate_integer_native80(value));
}
float local_player_distance(const float *point) {
    return cars::local_player_distance(point);
}
bool heading_allows_relocation(void *target) {
    return world::camera::route_math::heading_at_least_half(target);
}
bool lateral_above_three_quarters(void *entity) {
    return world::camera::route_math::lateral_above_three_quarters(entity);
}
} // namespace nfs::game::runtime::ai::police_math
