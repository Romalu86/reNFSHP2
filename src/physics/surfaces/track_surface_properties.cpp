#include "nfs/physics/solver/solver_runtime.hpp"
namespace nfs::game::runtime::physics::pending {

void *surface_properties(int surface) {
    return saved::surface_parameters + ((surface >= 0 && surface < 23) ? surface * 48 : 0);
}
} // namespace nfs::game::runtime::physics::pending
