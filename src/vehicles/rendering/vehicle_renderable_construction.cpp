#include "nfs/core/errors/assertions.hpp"
#include "vehicle_renderable_runtime.hpp"
#include "vehicle_shader_parameters.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/core/lifecycle/system_destruction.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/network/session/network_virtual.hpp"
#include <cstring>
#include <initializer_list>
namespace nfs::game::world::vehicle_render {
Dword damage_draw_state = 0;

void *construct_actor_channels(void *owner) {
    at<Dword>(owner, 0) = graphics::unresolved::camera_state;
    at<Dword>(owner, 16) = at<Dword>(owner, 12) = at<Dword>(owner, 4) = 0;
    at<Byte>(owner, 8) = at<Byte>(owner, 9) = 1;
    return owner;
}
// Complete RET.
void destroy_animation_channel(void *) {
}

void destroy_actor_channels(void *owner) {
    if (void *channels = at<void *>(owner, 12)) {
        auto *allocation = static_cast<Dword *>(channels) - 1;
        lifecycle::construction::objects::destroy_array(channels, 20, int(*allocation),
                                                        destroy_animation_channel);
        kernel::memory_free(allocation);
    }
    at<Dword>(owner, 12) = at<Dword>(owner, 16) = 0;
}

void *construct_damage_owner(void *owner) {
    construct_actor_channels(owner);
    at<Byte>(owner, 20) = 0;
    at<Dword>(owner, 52) = Dword(-1);
    at<Dword>(owner, 76) = at<Dword>(owner, 64) = 0;
    damage_draw_state = 0;
    return owner;
}

void destroy_damage_owner(void *owner) {
    destroy_actor_channels(owner);
}

void *construct_light_owner(void *owner) {
    lifecycle::construction::render_state(static_cast<Dword *>(owner));
    at<Byte>(owner, 748) = 0;
    for (unsigned field : {68u, 764u, 768u, 744u})
        at<Dword>(owner, field) = 0;
    return owner;
}

void destroy_light_owner(void *owner) {
    graphics::geoprim::base_destroy(static_cast<Dword *>(owner));
}

void *construct_driver_owner(void *owner) {
    at<Byte>(owner, 13) = 1;
    at<Dword>(owner, 28) = 1;
    at<Dword>(owner, 20) = at<Dword>(owner, 36) = 0;
    at<float>(owner, 32) = .5f;
    at<Dword>(owner, 16) = graphics::unresolved::camera_state;
    at<Byte>(owner, 12) = 0;
    at<Dword>(owner, 44) = Dword(-1);
    at<Dword>(owner, 56) = at<Dword>(owner, 52) = 0;
    at<Byte>(owner, 48) = 0;
    return owner;
}
// Complete RET.
void destroy_driver_owner(void *) {
}

void *construct_shadow_owner(void *owner) {
    at<Dword>(owner, 4) = at<Dword>(owner, 0) = at<Dword>(owner, 16) = at<Dword>(owner, 12) = 0;
    return owner;
}
// Complete RET.
void destroy_shadow_owner(void *) {
}

void *construct_attachment_pose(void *owner) {
    at<Dword>(owner, 0) = 0;
    at<Dword>(owner, 4) = Dword(-1);
    return owner;
}
// Complete RET.
void destroy_attachment_pose(void *) {
}

void *construct_attachment_owner(void *owner) {
    lifecycle::construction::construct_array(offset(owner, 48), 8, 45, construct_attachment_pose,
                                             destroy_attachment_pose);
    at<Dword>(owner, 408) = 0;
    std::memset(owner, 0, 45);
    return owner;
}

void destroy_attachment_owner(void *owner) {
    lifecycle::construction::objects::destroy_array(offset(owner, 48), 8, 45,
                                                    destroy_attachment_pose);
}

void *construct_body_interface(void *owner) {
    at<Dword>(owner, 1848) = 0;
    at<Byte>(owner, 1852) = 0;
    at<Dword>(owner, 1856) = 0;
    return owner;
}
// Complete RET.
void destroy_body_interface(void *) {
}
// Complete RET in the base interface table.
void __fastcall release_base_body(void *, void *) {
}
const Dword body_interface_methods[6] = {
    reinterpret_cast<Dword>(
        ::nfs::assertions::pure_virtual_call),
    reinterpret_cast<Dword>(
        ::nfs::assertions::pure_virtual_call),
    reinterpret_cast<Dword>(
        ::nfs::assertions::pure_virtual_call),
    reinterpret_cast<Dword>(
        ::nfs::assertions::pure_virtual_call),
    reinterpret_cast<Dword>(
        ::nfs::assertions::pure_virtual_call),
    reinterpret_cast<Dword>(release_base_body)};

void *construct_body_renderer(void *owner) {
    construct_body_interface(offset(owner, 36));
    at<Dword>(owner, 2132) = at<Dword>(owner, 2136) = at<Dword>(owner, 2140) = 0;
    std::memset(offset(owner, 2076), 0, 16);
    at<float>(owner, 4) = at<float>(owner, 12) = at<float>(owner, 2068) = 40;
    at<Dword>(owner, 2132) = 0;
    at<float>(owner, 2136) = 1;
    at<const Dword *>(owner, 0) = body_interface_methods;
    at<float>(owner, 8) = 1;
    at<float>(owner, 2064) = 3;
    at<Dword>(owner, 28) = 0;
    at<float>(owner, 2120) = 100;
    at<float>(owner, 2092) = at<float>(owner, 2096) = at<float>(owner, 2100) = 1;
    at<Dword>(owner, 2104) = at<Dword>(owner, 2108) = 0;
    at<Byte>(owner, 2072) = 0;
    at<Dword>(owner, 2052) = at<Dword>(owner, 2140) = 0;
    at<float>(owner, 2056) = 1.2000000476837158203125f;
    at<float>(owner, 2060) = 4.5f;
    return owner;
}

void destroy_body_renderer(void *owner) {
    at<const Dword *>(owner, 0) = body_interface_methods;
    destroy_body_interface(offset(owner, 36));
}

void __fastcall release_body_texture(void *owner, void *) {
    if (Dword *texture = at<Dword *>(owner, 2340)) {
        graphics::tar::destroy(texture);
        eagl::deallocator(texture, 48);
    }
    at<Dword>(owner, 2340) = 0;
}

void __fastcall body_model_bounds(void *owner, void *, Dword *minimum, Dword *maximum) {
    void *model = at<void *>(owner, 28);
    std::memcpy(minimum, offset(model, 108), 12);
    std::memcpy(maximum, offset(model, 124), 12);
}
// Complete RET4; the stack word is ignored.
void __fastcall finish_body_models(void *, void *, Dword) {
}
// Complete RET.
void __fastcall finish_body_frame(void *, void *) {
}
const Dword body_renderer_methods[6] = {
    reinterpret_cast<Dword>(pending::load_body_models),
    reinterpret_cast<Dword>(pending::prepare_body_shader),
    reinterpret_cast<Dword>(release_body_texture),
    reinterpret_cast<Dword>(body_model_bounds),
    reinterpret_cast<Dword>(finish_body_models),
    reinterpret_cast<Dword>(finish_body_frame)};

void *construct_body_materials(void *owner) {
    construct_body_renderer(owner);
    for (unsigned field = 2144; field <= 2200; field += 4)
        at<Dword>(owner, field) = 0;
    lifecycle::construction::render_state(reinterpret_cast<Dword *>(offset(owner, 2204)));
    try {
        lifecycle::construction::render_state(reinterpret_cast<Dword *>(offset(owner, 2272)));
    } catch (...) {
        graphics::geoprim::base_destroy(reinterpret_cast<Dword *>(offset(owner, 2204)));
        destroy_body_renderer(owner);
        throw;
    }
    for (unsigned field : {2368u, 2400u, 2384u})
        std::memset(offset(owner, field), 0, 16);
    at<const Dword *>(owner, 0) = body_renderer_methods;
    for (unsigned field : {2340u, 2344u, 28u, 32u, 2144u, 2160u, 2164u, 2180u, 2348u,
                           2356u, 2352u, 2360u})
        at<Dword>(owner, field) = 0;
    for (float &value : shader_parameters.hot_specular_colour)
        value = 1;
    for (unsigned field : {2416u, 2432u})
        for (unsigned i = 0; i < 4; ++i)
            at<float>(owner, field + i * 4) = 1;
    at<Byte>(owner, 2448) = at<Byte>(owner, 2449) = 0;
    at<Dword>(owner, 2452) = 0;
    return owner;
}

void destroy_body_materials(void *owner) {
    at<const Dword *>(owner, 0) = body_renderer_methods;
    graphics::geoprim::base_destroy(reinterpret_cast<Dword *>(offset(owner, 2272)));
    graphics::geoprim::base_destroy(reinterpret_cast<Dword *>(offset(owner, 2204)));
    destroy_body_renderer(owner);
}
} // namespace nfs::game::world::vehicle_render
namespace nfs::game::world::pending {

void *construct_vehicle_renderable(void *owner) {
    using namespace vehicle_render;
    unsigned stage = 0;
    try {
        construct_damage_owner(owner);
        stage = 1;
        construct_light_owner(offset(owner, 80));
        stage = 2;
        construct_driver_owner(offset(owner, 856));
        stage = 3;
        construct_body_materials(offset(owner, 1004));
        stage = 4;
        construct_shadow_owner(offset(owner, 3460));
        stage = 5;
        construct_attachment_owner(offset(owner, 3480));
    } catch (...) {
        if (stage >= 5)
            destroy_shadow_owner(offset(owner, 3460));
        if (stage >= 4)
            destroy_body_materials(offset(owner, 1004));
        if (stage >= 3)
            destroy_driver_owner(offset(owner, 856));
        if (stage >= 2)
            destroy_light_owner(offset(owner, 80));
        if (stage >= 1)
            destroy_damage_owner(owner);
        throw;
    }
    at<Dword>(owner, 4056) = at<Dword>(owner, 992) = at<Dword>(owner, 996) = 0;
    std::memset(offset(owner, 3892), 0, 12);
    constexpr float identity[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    std::memcpy(offset(owner, 3904), identity, 36);
    for (unsigned field : {3940u, 3944u, 3948u, 3952u, 3964u, 4040u, 4044u, 4056u, 3976u})
        at<Dword>(owner, field) = 0;
    for (unsigned field : {3968u, 3969u, 3960u, 4052u})
        at<Byte>(owner, field) = 0;
    std::memset(offset(owner, 3980), 0, 60);
    at<float>(owner, 4048) = 2.5f;
    return owner;
}

void destroy_vehicle_renderable(void *owner) {
    using namespace vehicle_render;
    destroy_attachment_owner(offset(owner, 3480));
    destroy_shadow_owner(offset(owner, 3460));
    destroy_body_materials(offset(owner, 1004));
    destroy_driver_owner(owner ? offset(owner, 856) : nullptr);
    destroy_light_owner(owner ? offset(owner, 80) : nullptr);
    destroy_damage_owner(owner);
}
} // namespace nfs::game::world::pending
