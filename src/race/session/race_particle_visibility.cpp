#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "race_particle_render.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include "../../math/precision/extended_float.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include <cmath>
namespace nfs::game::runtime::particles::lifecycle {
using kernel::numeric::Extended;
float near_fade_inverse;

static void initialize_particle_fade() {
    const auto format = kernel::numeric::current_format();
    near_fade_inverse = float(Extended(1.f, format) /
                               (Extended(1.5f, format) - Extended(1.f, format)));
}
namespace {
NFS_KERNEL_INITIALIZER(startup_initialize_particle_fade, ".CRT$XCU_NFS_00690170",
                       initialize_particle_fade);
}

void collect_particle_groups(void *owner, void *camera) {
    collect_visible_particles(at<void *>(owner, 4), camera);
}
} // namespace nfs::game::runtime::particles::lifecycle
