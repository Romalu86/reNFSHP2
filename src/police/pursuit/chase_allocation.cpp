#include "police_chase_x87.hpp"
#include "../../network/session/network_pursuit_runtime.hpp"
#include "../arrest/arrest_cinematic_runtime.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {

void *allocate_police_chase(void *owner, void *cop, void *target, Byte forced_announcement) {
    return police_frame::allocate_police_chase_retained(owner, cop, target,
                                                         forced_announcement);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
