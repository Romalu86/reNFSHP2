#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/world/resources/runtime_resources.hpp"

namespace nfs::game::runtime::particles::lifecycle {
void release_car_textures() {
    for (unsigned address = 7216656; address < 7217168; address += 16)
        if (Dword *texture = state<Dword *>(address)) {
            graphics::tar::destroy(texture);
            eagl::deallocator(texture, 48);
        }
    kernel::memory_zero(&state<Byte>(7216648), 512);
    if (void *image = state<void *>(7217164)) {
        kernel::memory_free(image);
        state<Dword>(7217164) = 0;
    }
    if (void *image = state<void *>(7217160)) {
        kernel::memory_free(image);
        state<Dword>(7217160) = 0;
    }
}

void initialize_car_effects() {
    reset_particle_buffers();
    const unsigned states[] = {7216576, 7216504, 7216432, 7216360};
    for (unsigned index = 0; index < 4; ++index) {
        Dword *render = &state<Dword>(states[index]);
        graphics::geoprim::primitive(render, 4);
        if ((index & 1) == 0) {
            graphics::geoprim::texture_enable(render, 1);
            graphics::geoprim::shading(render, 1);
            graphics::geoprim::texture_coordinates(render, 1);
        } else {
            graphics::geoprim::shading(render, 1);
            graphics::geoprim::texture_coordinates(render, 1);
            graphics::geoprim::texture_enable(render, 1);
        }
        graphics::geoprim::alpha_test(render, 0);
        graphics::geoprim::alpha_blend_mode(render, (index & 1) ? 2 : 1);
        graphics::geoprim::transparency(render, 1);
        graphics::geoprim::culling(render, 0);
        graphics::geoprim::depth_method(render, index < 2 ? 2 : 8);
        graphics::geoprim::transparency(render, 1);
    }
    pending::load_car_particle_textures();
    pending::load_car_particle_definitions();
}
} // namespace nfs::game::runtime::particles::lifecycle
