#include "nfs/graphics/particles/particle_lifecycle.hpp"
namespace nfs::game::runtime::particles::lifecycle {

void release_texture_scratch(TextureScratch *scratch) {
    scratch->owner->fields[13] = 0;
}

void *texture_scratch_data(TextureScratch *scratch) {
    return reinterpret_cast<void *>(scratch->owner->fields[11]);
}

Dword texture_scratch_capacity(TextureScratch *scratch) {
    return scratch->owner->fields[12];
}
} // namespace nfs::game::runtime::particles::lifecycle
