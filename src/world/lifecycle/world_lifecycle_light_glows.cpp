#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/network/session/network_virtual.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstring>
namespace nfs::game::world {


void destroy_light_glow(void *object) {
    if (Dword *texture = at<Dword *>(object, 220)) {
        graphics::tar::destroy(texture);
        eagl::deallocator(texture, 48);
        at<Dword *>(object, 220) = nullptr;
    }
}

void bind_light_glows(void *owner, Dword *definitions, Dword *textures) {
    int instances = int(definitions[0]);
    at<int>(owner, 76) = instances;
    at<Dword>(owner, 68) = definitions[2];
    auto *cookie = static_cast<Dword *>(files::archives::allocate_entry(
        int(Dword(instances) * 248 + 4), "TrackLightGlow Instance List", lifecycle::level_pool));
    void *objects = nullptr;
    if (cookie) {
        objects = cookie + 1;
        *cookie = Dword(instances);
        try {
            lifecycle::construction::construct_array(objects, 248, instances, construct_light_glow,
                                                     destroy_light_glow);
        } catch (...) {
            lifecycle::construction::network::entries::release_payload(cookie);
            throw;
        }
    }
    at<void *>(owner, 80) = objects;
    static constexpr float texture_coordinates[24] = {0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1,
                                                      1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1};
    for (int index = 0; index < at<int>(owner, 76); ++index) {
        void *object = offset(at<void *>(owner, 80), index * 248);
        Byte *entry = reinterpret_cast<Byte *>(definitions[1]) + index * 20;
        at<Dword>(object, 4) = 1065353216;
        at<Dword>(object, 244) = 1065353216;
        Dword size = at<Dword>(entry, 12);
        at<Dword>(object, 16) = size;
        at<Dword>(object, 12) = size;
        at<Dword>(object, 0) = at<Dword>(entry, 4);
        at<Byte>(object, 228) = 0;
        at<Byte>(object, 224) = entry[0];
        at<Byte>(object, 225) = entry[1];
        at<Byte>(object, 226) = entry[2];
        Byte alpha = Byte(255 - entry[3]);
        at<Byte>(object, 227) = alpha;
        at<Dword>(object, 212) =
            (Dword(alpha) << 24) | (Dword(entry[0]) << 16) | (Dword(entry[1]) << 8) | entry[2];
        at<float>(object, 232) =
            float(static_cast<long double>(alpha) * 0.0039215688593685626983642578125f);
        std::memcpy(offset(object, 116), texture_coordinates, sizeof(texture_coordinates));
        Dword texture_offset = at<Dword>(textures, 20 + at<Dword>(entry, 16) * 8);
        Dword *image = reinterpret_cast<Dword *>(reinterpret_cast<Dword>(textures) + texture_offset);
        auto *texture = static_cast<Dword *>(
            lifecycle::allocate(48, "TrackLightGlow TAR", lifecycle::level_pool));
        if (texture) {
            try {
                graphics::tar::construct_shape(texture, image);
            } catch (...) {
                kernel::memory_free(texture);
                throw;
            }
        }
        at<Dword *>(object, 220) = texture;
        at<float>(object, 216) = float(static_cast<long double>(
            std::int32_t(graphics::unresolved::camera_state)) * 0.01666666753590106964111328125f);
        at<Dword>(object, 240) = 0;
        at<Byte>(object, 8) = 0;
        at<Byte>(object, 236) = 1;
    }
    int lights = at<int>(owner, 68);
    at<void *>(owner, 72) = files::archives::allocate_entry(
        int(Dword(lights) * 28), "TrackLightGlow Light List", lifecycle::level_pool);
    for (int index = 0; index < lights; ++index) {
        void *light = offset(at<void *>(owner, 72), index * 28);
        Dword *source = reinterpret_cast<Dword *>(definitions[3]) + index * 33;
        at<Byte>(light, 0) = 1;
        at<Dword>(light, 8) = source[0];
        void *object = offset(at<void *>(owner, 80), source[0] * 248);
        for (unsigned vertex = 0; vertex < 6; ++vertex) {
            Dword *destination = reinterpret_cast<Dword *>(offset(object, 20 + vertex * 16));
            destination[0] = source[5 + vertex * 3];
            destination[1] = source[6 + vertex * 3];
            destination[2] = source[7 + vertex * 3];
            destination[3] = 1065353216;
        }
        Dword saved[4];
        std::memcpy(saved, offset(object, 52), 16);
        std::memcpy(offset(object, 52), offset(object, 68), 16);
        std::memcpy(offset(object, 68), saved, 16);
        at<Dword>(light, 12) = source[1];
        at<Dword>(light, 16) = source[2];
        at<Dword>(light, 20) = source[3];
        at<Dword>(light, 24) = source[4];
    }
    auto *material = static_cast<Dword *>(owner);
    graphics::geoprim::shading(material, 1);
    graphics::geoprim::texture_enable(material, 1);
    graphics::geoprim::alpha_test(material, 0);
    graphics::geoprim::alpha_blend_mode(material, 2);
    graphics::geoprim::transparency(material, 1);
    graphics::geoprim::culling(material, 0);
    graphics::geoprim::depth_method(material, 4);
    at<Byte>(owner, 84) = 1;
}

void unload_light_glows(void *owner) {
    int count = at<int>(owner, 76);
    Dword *texture = at<Dword *>(owner, 80);
    for (int index = 0; index < count; ++index) {
        void *object = offset(texture, index * 248);
        if (Dword *image = at<Dword *>(object, 220)) {
            graphics::tar::destroy(image);
            eagl::deallocator(image, 48);
            at<Dword *>(object, 220) = nullptr;
        }
    }
    if (void *objects = at<void *>(owner, 80)) {
        auto *cookie = static_cast<Dword *>(objects) - 1;
        lifecycle::construction::objects::destroy_array(objects, 248, int(*cookie),
                                                        destroy_light_glow);
        lifecycle::construction::network::entries::release_payload(cookie);
        at<void *>(owner, 80) = nullptr;
    }
    if (void *lights = at<void *>(owner, 72)) {
        lifecycle::construction::network::entries::release_payload(lights);
        at<void *>(owner, 72) = nullptr;
    }
    at<Dword>(owner, 100) = 0;
    at<Byte>(owner, 84) = 0;
}
} // namespace nfs::game::world
