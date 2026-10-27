#include "../../ai/driving/ai_route_planner.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"

namespace nfs::game::runtime {

extern const float turns_per_radian = 0.15915493667125702f;
}

namespace nfs::game::runtime::ai::entities::pending {
using kernel::numeric::Extended;

Extended direction_turns(float x, float z) {
    const auto format = kernel::numeric::current_format();
    unsigned char native_value[10];
    direction_x87::direction_turns_native80(x, z, native_value);
    return Extended::from_native80(native_value, format);
}

Extended steer_toward_position(void *entity, const float *target, void *controls) {
    const auto format = kernel::numeric::current_format();
    unsigned char native_value[10];
    direction_x87::steer_toward_position_native80(entity, target, controls, native_value);
    return Extended::from_native80(native_value, format);
}
} // namespace nfs::game::runtime::ai::entities::pending
