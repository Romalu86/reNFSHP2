#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "camera_controller_runtime.hpp"
namespace nfs::game::world::camera {
const float up_vector[3] = {0.f, 1.f, 0.f};
const float *up_direction{};

void initialize_up_direction() {
    up_direction = up_vector;
}
} // namespace nfs::game::world::camera

NFS_KERNEL_INITIALIZER(startup_initialize_up_direction, ".CRT$XCU_NFS_006902C8",
                       nfs::game::world::camera::initialize_up_direction);
