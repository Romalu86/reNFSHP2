#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
#include <cstdio>
namespace nfs::game::runtime::particles::pending {
void fsh_directory_name(Dword *, int, Dword *);
}
namespace nfs::game::runtime::particles::lifecycle::pending {
namespace {
void append_archive_textures(unsigned image_address) {
    Dword *image = state<Dword *>(image_address);
    if (!image)
        return;
    const int count = static_cast<int>(image[2]);
    for (int index = 0; index < count; ++index) {
        auto *shape =
            reinterpret_cast<Dword *>(reinterpret_cast<Dword>(image) + image[5 + index * 2]);
        if (shape) {
            auto *texture = static_cast<Dword *>(
                nfs::lifecycle::allocate(48, "CarParticle::mArt", nfs::lifecycle::level_pool));
            if (texture) {
                try {
                    graphics::tar::construct_shape(texture, shape);
                } catch (...) {
                    gui::application_release(texture);
                    throw;
                }
            }
            Dword entry = state<Dword>(7156728) * 16;
            state<Dword *>(7216656 + entry) = texture;
            particles::pending::fsh_directory_name(state<Dword *>(image_address), index,
                                                   &state<Dword>(7216648 + entry));
            ++state<Dword>(7156728);
            image = state<Dword *>(image_address);
        }
    }
}
} // namespace

void load_car_particle_textures() {
    release_car_textures();
    kernel::memory_zero(&state<Byte>(7216648), 512);
    state<Dword>(7156728) = 0;
    char filename[260];
    std::sprintf(filename, "%sParticle\\particle.fsh", reinterpret_cast<char *>(system(4)) + 4);
    state<Dword>(7156728) = 0;
    TextureScratch scratch;
    acquire_texture_scratch(&scratch, &graphics::textures::database);
    try {
        if (files::file::load_into(filename, texture_scratch_data(&scratch),
                                   int(texture_scratch_capacity(&scratch))))
            state<void *>(7217160) =
                convert_fsh_image(&graphics::textures::database, texture_scratch_data(&scratch),
                                  nfs::lifecycle::level_pool | 1024);
        append_archive_textures(7217160);
        std::sprintf(filename, "%sParticle\\carfx.fsh", reinterpret_cast<char *>(system(4)) + 4);
        if (files::file::load_into(filename, texture_scratch_data(&scratch),
                                   int(texture_scratch_capacity(&scratch))))
            state<void *>(7217164) =
                convert_fsh_image(&graphics::textures::database, texture_scratch_data(&scratch),
                                  nfs::lifecycle::level_pool | 1024);
        append_archive_textures(7217164);
    } catch (...) {
        release_texture_scratch(&scratch);
        throw;
    }
    release_texture_scratch(&scratch);
}
} // namespace nfs::game::runtime::particles::lifecycle::pending
