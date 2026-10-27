#include "police_chase_x87.hpp"
#include "chase_runtime.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {

void update_police_chase(Dword *owner, void *chase) {
    police_frame::update_police_chase_retained(owner, chase);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
