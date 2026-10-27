#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include <cstdlib>

namespace nfs::game::runtime::particles::lifecycle {

TextureScratch *acquire_texture_scratch(TextureScratch *scratch, graphics::TextureReader *owner) {
    owner->fields[13] = 1;
    owner->fields[12] = 5242880;
    scratch->owner = owner;
    if (void *previous = reinterpret_cast<void *>(owner->fields[11]))
        std::free(previous);
    void *data = std::malloc(5242880);
    owner->fields[11] = reinterpret_cast<Dword>(data);
    if (!data)
        owner->fields[12] = 0;
    return scratch;
}
namespace {
struct TextureScratchScope {
    TextureScratch scratch;
    TextureScratchScope() {
        acquire_texture_scratch(&scratch, &graphics::textures::database);
    }
    ~TextureScratchScope() {
        release_texture_scratch(&scratch);
    }
};
} // namespace

Dword load_fsh_file(const char *name, Dword *count, Dword **output, Dword pool) {
    *count = 0;
    *output = nullptr;
    TextureScratchScope scope;
    if (files::file::try_load_into(name, texture_scratch_data(&scope.scratch),
                                   static_cast<int>(texture_scratch_capacity(&scope.scratch))))
        *output = static_cast<Dword *>(pending::convert_fsh_image(
            &graphics::textures::database, texture_scratch_data(&scope.scratch), pool));
    if (!*output)
        return 0;
    *count = (*output)[2];
    return 1;
}
} // namespace nfs::game::runtime::particles::lifecycle
namespace nfs::game::runtime::particles::lifecycle::pending {
void *load_fsh(const char *name, Dword *count, Dword **output, Dword pool) {
    return reinterpret_cast<void *>(load_fsh_file(name, count, output, pool));
}
} // namespace nfs::game::runtime::particles::lifecycle::pending
