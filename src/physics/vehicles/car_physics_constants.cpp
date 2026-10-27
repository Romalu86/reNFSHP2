#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "car_physics_frame.hpp"
namespace nfs::game::runtime::physics::car_frame {
float gravity = 9.81000041961669921875f;
float inverse_gravity{};
 
void initialize_inverse_gravity() {
    inverse_gravity = float(number(1) / number(gravity));
}
} // namespace nfs::game::runtime::physics::car_frame
NFS_KERNEL_INITIALIZER(startup_initialize_inverse_gravity, ".CRT$XCU_NFS_0069002C",
                       nfs::game::runtime::physics::car_frame::initialize_inverse_gravity);
