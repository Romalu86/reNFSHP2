#include "car_physics_frame.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
namespace nfs::game::runtime::physics::car_frame {
// Public compatibility aggregate stays within MSVC.
Extended wheel_force_scale(void *wheel) {
    const auto format = kernel::numeric::current_format();
    unsigned char native_value[10];
    x87::wheel_force_scale_native80(wheel, native_value);
    return Extended::from_native80(native_value, format);
}
} // namespace nfs::game::runtime::physics::car_frame
