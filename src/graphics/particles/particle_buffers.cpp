#include "nfs/world/resources/runtime_resources.hpp"
namespace nfs::game::runtime {
// Exact BSS write footprints, not inferred particle element types.
Dword particle_buffer_count{};
alignas(4) Byte particle_links[160]{}, particle_states[3600]{},
    particle_geometry[55800]{};

void reset_particle_buffers() {
    particle_buffer_count = 0;
    kernel::memory_zero(particle_geometry, 55800);
    kernel::memory_zero(particle_states, 3600);
    kernel::memory_zero(particle_links, 160);
}
} // namespace nfs::game::runtime
