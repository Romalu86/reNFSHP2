#include "nfs/physics/solver/solver_runtime.hpp"
namespace nfs::game::runtime::physics::pending {
// Leave bytes 52h..53h untouched.
void *construct_ground_contact(void *contact) {
    for (unsigned offset = 0; offset <= 72; offset += 4)
        at<Dword>(contact, offset) = 0;
    at<std::uint16_t>(contact, 76) = at<std::uint16_t>(contact, 78) =
        at<std::uint16_t>(contact, 80) = 0;
    return contact;
}
} // namespace nfs::game::runtime::physics::pending
