#include "ai_path_frame_x87.hpp"
#include "ai_car_behavior.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
namespace nfs::game::runtime::ai::cars {
using kernel::numeric::Extended;

Extended steer_car_to_position(void *entity, const float *position, void *controls) {
    alignas(4) unsigned char retained[10];
    path_frame::steering_native80(entity, position, controls, retained);
    return Extended::from_native80(retained, kernel::numeric::current_format());
}

void __fastcall car_recover_motion(void *entity, void *, Dword controls_word) {
    path_frame::recover_motion(entity, controls_word);
}
} // namespace nfs::game::runtime::ai::cars
