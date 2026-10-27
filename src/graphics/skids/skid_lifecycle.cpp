#include "nfs/graphics/skids/skid_lifecycle.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include <cstdio>

namespace nfs::game::runtime::skids::lifecycle {

void load_level(Dword *owner) {
    owner[3] = 0;
    owner[1] = 0;
    owner[2] = 0;
    owner[4] = 0;
    if (!graphics::device)
        return;
    initialize_free_list();
    auto *array = static_cast<Dword *>(
        nfs::lifecycle::allocate(128, "SKID_ARRAY", nfs::lifecycle::track_pool));
    if (array) {
        try {
            array = construct_array(array);
        } catch (...) {
            game::runtime::track::pending::release_named(
                array, "SKID_ARRAY", nfs::lifecycle::track_pool);
            throw;
        }
    }
    owner[3] = reinterpret_cast<Dword>(array);
    Dword *render = reinterpret_cast<Dword *>(bytes(owner) + 20);
    graphics::geoprim::primitive(render, 4);
    graphics::geoprim::shading(render, 1);
    graphics::geoprim::culling(render, 1);
    graphics::geoprim::texture_enable(render, 1);
    graphics::geoprim::depth_method(render, 4);
    graphics::geoprim::transparency(render, 1);
    graphics::geoprim::alpha_blend_mode(render, 1);
    graphics::geoprim::alpha_test(render, 0);
    const Dword option = network::packets::rule_value(system(72), 6);
    graphics::geoprim::cull_direction(render, option == 2 || option == 3);
    auto *primitive = static_cast<Dword *>(
        nfs::lifecycle::allocate(84, "SKID_PRIM", nfs::lifecycle::track_pool));
    if (primitive) {
        try {
            auto *method = static_cast<Dword *>(eagl::allocator(44, "EAGL::Rendermethod new"));
            primitive[0] = reinterpret_cast<Dword>(
                method ? pending::construct_render_method(method, pending::render_method_definition)
                       : nullptr);
            const char *names[] = {"EAGL::ViewPort::gpModelViewProjectionMatrix",
                                   "EAGL::ViewPort::gpModelMatrix", "EAGL::ViewPort::gpViewMatrix",
                                   "EAGL::ViewPort::gpProjectionMatrix",
                                   "&EAGL::RenderMethodConstants::gFogParameters"};
            for (unsigned index = 0; index < 5; ++index) {
                Byte found;
                primitive[4 + index * 2] =
                    eagl::pool_lookup(&eagl::external_symbols, names[index], &found);
                primitive[3 + index * 2] = 1;
            }
        } catch (...) {
            game::runtime::track::pending::release_named(
                primitive, "SKID_PRIM", nfs::lifecycle::track_pool);
            throw;
        }
    }
    owner[4] = reinterpret_cast<Dword>(primitive);
    char name[260];
    std::sprintf(name, "%sskidmarks.fsh", bytes(system(4)) + 2084);
    particles::lifecycle::TextureScratch scratch;
    particles::lifecycle::acquire_texture_scratch(&scratch, &graphics::textures::database);
    try {
        if (files::file::try_load_into(
                name, particles::lifecycle::texture_scratch_data(&scratch),
                static_cast<int>(particles::lifecycle::texture_scratch_capacity(&scratch))))
            owner[92 / 4] =
                reinterpret_cast<Dword>(particles::lifecycle::pending::convert_fsh_image(
                    &graphics::textures::database,
                    particles::lifecycle::texture_scratch_data(&scratch),
                    nfs::lifecycle::cars_pool | 1024));
        Dword *image = reinterpret_cast<Dword *>(owner[92 / 4]);
        owner[96 / 4] = 0;
        int height = 256;
        if (image) {
            auto *shape = bytes(image) + at<Dword>(image, 20);
            if (shape) {
                height = at<std::int16_t>(shape, 4);
                auto *texture = static_cast<Dword *>(
                    nfs::lifecycle::allocate(48, "skidTex", nfs::lifecycle::cars_pool));
                if (texture) {
                    try {
                        texture = graphics::tar::construct_shape(
                            texture, reinterpret_cast<Dword *>(shape));
                    } catch (...) {
                        game::runtime::track::pending::release_named(
                            texture, "skidTex", nfs::lifecycle::cars_pool);
                        throw;
                    }
                }
                owner[96 / 4] = reinterpret_cast<Dword>(texture);
            }
        }
        int row = height;
        for (int index = 0; index < 6; ++index) {
            at<float>(owner, 100 + index * 4) = static_cast<float>(index) * 0.125f;
            at<float>(owner, 124 + index * 4) =
                (static_cast<float>(row) * 0.125f - 1.0f) / static_cast<float>(height);
            row += height;
        }
        pending::allocate_vertices(reinterpret_cast<Dword *>(owner[3]), 2000, 4,
                                   reinterpret_cast<Dword *>(owner[4]), render,
                                   reinterpret_cast<Dword *>(owner[96 / 4]), 136,
                                   "SKIDS_DRAWARRAY_VTX_DATA", nfs::lifecycle::track_pool);
        if (auto *indices = at<std::uint16_t *>(reinterpret_cast<void *>(owner[3]), 124))
            for (unsigned index = 0; index < 2000; ++index)
                indices[index] = static_cast<std::uint16_t>(index);
        graphics::immediate::model_builder_matrix(reinterpret_cast<Dword *>(owner[3]),
                                                  identity_matrix);
    } catch (...) {
        particles::lifecycle::release_texture_scratch(&scratch);
        throw;
    }
    particles::lifecycle::release_texture_scratch(&scratch);
}
} // namespace nfs::game::runtime::skids::lifecycle
