#include "../driving/ai_drive_x87.hpp"
#include "../driving/ai_driver_runtime.hpp"
#include "../../graphics/cameras/camera_director_runtime.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "../../math/vectors/retained_vector.hpp"
namespace nfs::game::runtime::ai::cars {

void __fastcall traffic_drive(void *entity, void *, Dword controls_word) {
    drive_frame::traffic_controls(entity, controls_word);
}
} // namespace nfs::game::runtime::ai::cars
