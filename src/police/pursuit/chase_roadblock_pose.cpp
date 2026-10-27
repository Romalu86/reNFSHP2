#include "police_chase_x87.hpp"
#include "chase_runtime.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {
using kernel::numeric::Extended;



Dword place_roadblock_cop(void *location, void *cop) {
    return police_frame::place_roadblock_cop_retained(location, cop);
}

void place_roadblock_spikes(void *location, void *spike) {
    police_frame::place_roadblock_spikes_retained(location, spike);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
