#include "../../police/pursuit/police_chase_x87.hpp"
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include <cstring>

namespace nfs::game::runtime::ai::lifecycle {

Byte assign_pursuer(void *entity, void *target) {
    return police_frame::assign_pursuer_retained(entity, target);
}
} // namespace nfs::game::runtime::ai::lifecycle
namespace nfs::game::runtime::ai::pending {
Byte assign_pursuer_entry(void *entity, void *target) {
    return lifecycle::assign_pursuer(entity, target);
}
} // namespace nfs::game::runtime::ai::pending
