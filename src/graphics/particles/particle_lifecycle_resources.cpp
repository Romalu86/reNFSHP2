#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/core/lifecycle/system_destruction.hpp"
#include "nfs/core/lifecycle/array_construction.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include <initializer_list>

namespace nfs::game::runtime::particles::lifecycle {

void *construct_render_item(void *item) {
    at<const Dword *>(item, 20) = pending::render_item_base_methods;
    at<const Dword *>(item, 20) = pending::render_item_methods;
    at<void *>(item, 24) = item;
    at<Dword>(item, 4) = at<Dword>(item, 8) = 4294967295u;
    return item;
}

void destroy_render_item(void *item) {
    at<const Dword *>(item, 20) = pending::render_item_base_methods;
}
namespace {
struct RenderItemsOwner {
    void *items;
    ~RenderItemsOwner() noexcept(false) {
        if (items)
            nfs::lifecycle::construction::objects::destroy_array(
                items, 28, 18, destroy_render_item);
    }
};
struct RenderStateOwner {
    Dword *state;
    ~RenderStateOwner() noexcept(false) {
        if (state)
            nfs::lifecycle::construction::destruction::render_state_member(state);
    }
};
}

Dword *construct_renderer(Dword *renderer) {
    void *items = bytes(renderer) + 12004;
    nfs::lifecycle::construction::construct_array(
        items, 28, 18, construct_render_item, destroy_render_item);
    RenderItemsOwner items_owner{items};
    auto *first = reinterpret_cast<Dword *>(bytes(renderer) + 12508);
    auto *second = reinterpret_cast<Dword *>(bytes(renderer) + 12576);
    nfs::lifecycle::construction::render_state(first);
    RenderStateOwner first_owner{first};
    nfs::lifecycle::construction::render_state(second);
    RenderStateOwner second_owner{second};
    for (Dword *state : {first, second}) {
        graphics::geoprim::shading(state, 0);
        graphics::geoprim::texture_enable(state, state == first ? 0 : 1);
        graphics::geoprim::transparency(state, 1);
        graphics::geoprim::alpha_blend_mode(state, 1);
        graphics::geoprim::depth_method(state, 4);
        graphics::geoprim::alpha_reference(state, 0);
        graphics::geoprim::alpha_method(state, 5);
        graphics::geoprim::alpha_test(state, 1);
    }
    renderer[0] = 0;
    for (unsigned index = 0; index < 18; ++index) {
        at<Dword>(renderer, 12008 + index * 28) = 4294967295u;
        at<Dword>(renderer, 12012 + index * 28) = 4294967295u;
    }
    second_owner.state = nullptr;
    first_owner.state = nullptr;
    items_owner.items = nullptr;
    return renderer;
}

void destroy_renderer(Dword *renderer) {
    RenderItemsOwner items_owner{bytes(renderer) + 12004};
    RenderStateOwner first_owner{reinterpret_cast<Dword *>(bytes(renderer) + 12508)};
    nfs::lifecycle::construction::destruction::render_state_member(
        reinterpret_cast<Dword *>(bytes(renderer) + 12576));
}

void release_pool_image(Dword *pool) {
    if (void *image = at<void *>(pool, 808))
        kernel::memory_free(image);
    at<Dword>(pool, 808) = 0;
}

void load_pool_file(Dword *pool, const char *name) {
    if (void *data = files::file::try_load(name, at<Dword>(pool, 812))) {
        particles::pending::load_pool_entry(pool, static_cast<Dword *>(data));
        kernel::memory_free(data);
    }
}

void load_level(Dword *owner) {
    auto *buffer = static_cast<Dword *>(
        nfs::lifecycle::allocate(72004, "particleBuf", gui::application_pool));
    Dword *constructed_buffer;
    try {
        constructed_buffer = buffer ? construct_particle_buffer(buffer) : nullptr;
    } catch (...) {
        track::pending::release_named(buffer, "particleBuf", gui::application_pool);
        throw;
    }
    owner[1] = reinterpret_cast<Dword>(constructed_buffer);
    auto *renderer = static_cast<Dword *>(
        nfs::lifecycle::allocate(12668, "particleRender", gui::application_pool));
    Dword *constructed_renderer;
    try {
        constructed_renderer = renderer ? construct_renderer(renderer) : nullptr;
    } catch (...) {
        track::pending::release_named(renderer, "particleRender", gui::application_pool);
        throw;
    }
    owner[2] = reinterpret_cast<Dword>(constructed_renderer);
    Dword count;
    Dword *image;
    pending::load_fsh(level_file("particle.fsh"), &count, &image, nfs::lifecycle::level_pool);
    auto *pool = static_cast<Dword *>(
        nfs::lifecycle::allocate(816, "levelParticleManager", nfs::lifecycle::level_pool));
    Dword *constructed_pool;
    try {
        constructed_pool = pool ? particles::construct_pool(pool, reinterpret_cast<Dword>(image),
                                                             nfs::lifecycle::level_pool)
                                : nullptr;
    } catch (...) {
        track::pending::release_named(pool, "levelParticleManager", nfs::lifecycle::level_pool);
        throw;
    }
    owner[3] = reinterpret_cast<Dword>(constructed_pool);
    load_pool_file(reinterpret_cast<Dword *>(owner[3]), level_file("particle.mpb"));
    particles::enable_emitters(reinterpret_cast<Dword *>(owner[3]));
    initialize_car_effects();
}

void unload_level(Dword *owner) {
    pending::diagnostic_message("particle system: start level deinit\n");
    Dword *pool = reinterpret_cast<Dword *>(owner[3]);
    release_pool_image(pool);
    if (pool) {
        particles::destroy_pool(pool);
        gui::application_release(pool);
    }
    owner[3] = 0;
    if (Dword *renderer = reinterpret_cast<Dword *>(owner[2])) {
        destroy_renderer(renderer);
        gui::application_release(renderer);
    }
    owner[2] = 0;
    gui::application_release(reinterpret_cast<void *>(owner[1]));
    owner[1] = 0;
    state<Dword>(7156732) = 0;
    kernel::memory_zero(&state<Byte>(7160496), 55800);
    release_car_textures();
    state<Dword>(7156728) = 0;
    kernel::memory_zero(&state<Byte>(7216648), 512);
    kernel::memory_zero(&state<Byte>(7151864), 4864);
    state<Dword>(7217168) = 0;
}
} // namespace nfs::game::runtime::particles::lifecycle
namespace nfs::game::runtime::ai::lifecycle::pending {
void clear_car_contact(Dword *car, int wheel) {
    particles::lifecycle::release_wheel_skid(car, wheel);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
namespace nfs::game::runtime::particles::lifecycle::pending {

Dword diagnostic_message(const char *) {
    return 0;
} // Complete release-build body; no logging is invented.
} // namespace nfs::game::runtime::particles::lifecycle::pending
