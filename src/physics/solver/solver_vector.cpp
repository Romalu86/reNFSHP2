#include "nfs/physics/solver/solver_runtime.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::game::runtime::physics::pending {
// The saved length squared and length are each rounded to float.
long double normalize_collision_entry(float *vector) {
    return hardware::normalize_collision(vector);
}
} // namespace nfs::game::runtime::physics::pending
