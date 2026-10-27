#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstdlib>
namespace nfs::game::runtime::physics::pending {
void reset_vehicle_entry(void *);
}
namespace nfs::game::runtime::physics {

void *construct_vehicle(void *state) {
    for (unsigned offset = 508; offset <= 528; offset += 4)
        at<Dword>(state, offset) = 0;
    for (unsigned offset = 536; offset <= 580; offset += 4)
        at<Dword>(state, offset) = 0;
    at<std::uint16_t>(state, 584) = at<std::uint16_t>(state, 586) =
        at<std::uint16_t>(state, 588) = 0;
    at<Dword>(state, 532) = 0;
    pending::reset_vehicle_entry(state);
    return state;
}
// Single RET.
void destroy_vehicle(void *) {
}

void destroy_vehicle_registry() {
    lifecycle::construction::objects::destroy_array(pending::vehicle_states, 736, 40,
                                                    destroy_vehicle);
}
// Registration is explicit, preserving the saved startup boundary.
void construct_vehicle_registry() {
    lifecycle::construction::construct_array(pending::vehicle_states, 736, 40,
                                             construct_vehicle, destroy_vehicle);
    std::atexit(destroy_vehicle_registry);
}
} // namespace nfs::game::runtime::physics

// Construct the vehicle state before any runtime reset.
NFS_KERNEL_INITIALIZER(startup_construct_vehicle_registry, ".CRT$XCU_NFS_00690030",
                       nfs::game::runtime::physics::construct_vehicle_registry);
