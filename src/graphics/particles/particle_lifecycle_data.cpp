#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/graphics/skids/skid_lifecycle.hpp"
#include <cstring>
namespace nfs::game::runtime {
extern Byte particle_links[160], particle_states[3600], particle_geometry[55800];
extern Dword particle_buffer_count;
} // namespace nfs::game::runtime
namespace nfs::game::runtime::particles::lifecycle {
#include "particle_lifecycle_data.inc"
// The two effect draw paths copy this saved matrix.
void initialize_effect_matrix() {
    std::memcpy(particle_suffix, skids::lifecycle::identity_matrix, 64);
}
Byte *state_address(unsigned address) {
    if (address >= 7216296)
        return particle_suffix + (address - 7216296);
    if (address >= 7160496)
        return runtime::particle_geometry + (address - 7160496);
    if (address >= 7156896)
        return runtime::particle_states + (address - 7156896);
    if (address >= 7156736)
        return runtime::particle_links + (address - 7156736);
    if (address >= 7156732)
        return reinterpret_cast<Byte *>(&runtime::particle_buffer_count) + (address - 7156732);
    return particle_prefix + (address - 7151864);
}
} // namespace nfs::game::runtime::particles::lifecycle
 
NFS_KERNEL_INITIALIZER(startup_initialize_effect_matrix, ".CRT$XCU_NFS_00690020",
                       nfs::game::runtime::particles::lifecycle::initialize_effect_matrix);
