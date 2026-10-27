#include "../../ai/driving/ai_drive_x87.hpp"
#include "cop_driving_runtime.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include <cmath>
namespace nfs::game::runtime::ai::cars {
using namespace physics::car_frame;

Byte steer_cop_pursuit(void *entity, Dword *controls, float speed_bias) {
    return drive_frame::pursuit_controls(entity, controls, speed_bias);
}
} // namespace nfs::game::runtime::ai::cars
