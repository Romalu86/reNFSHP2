#include "car_physics_frame.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
namespace nfs::game::runtime::physics::car_frame::legacy {
// Discard the existing aggregate results entirely inside the MSVC boundary.
void discard_rolling_resistance(void *car) {
    apply_wheel_rolling_resistance(car);
}
void discard_clutch_limit() {
    car_clutch_limit();
}
} // namespace nfs::game::runtime::physics::car_frame::legacy
