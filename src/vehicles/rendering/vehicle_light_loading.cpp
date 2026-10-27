#include "vehicle_renderable_resources.hpp"
#include "vehicle_attachment_transforms.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstdio>
#include <cstring>
#include <initializer_list>
namespace nfs::game::runtime::particles::pending {
Dword *find_fsh_image(Dword *, const char *);
}
namespace nfs::game::world::vehicle_render {

void load_vehicle_glows(void *owner, ini::Reader *reader) {
    ini::get_int(reader, "lightglows", "glow_num", &at<int>(owner, 68));
    Dword count = at<Dword>(owner, 68);
    if (!count)
        return;
    auto *allocation = static_cast<Dword *>(lifecycle::allocate(
        count * 248 + 4, "VehicleLightGlow Instance List", lifecycle::cars_pool));
    void *glows = nullptr;
    if (allocation) {
        *allocation = count;
        try {
            lifecycle::construction::construct_array(allocation + 1, 248, int(count),
                                                     world::construct_light_glow,
                                                     world::destroy_light_glow);
        } catch (...) {
            kernel::memory_free(allocation);
            throw;
        }
        glows = allocation + 1;
    }
    at<void *>(owner, 744) = glows;
    auto X = [](float value) { return static_cast<long double>(value); };
    constexpr Dword coordinates[24] = {0,          0,          1065353216, 1065353216, 0,
                                      1065353216, 1065353216, 1065353216, 1065353216, 1065353216,
                                      1065353216, 1065353216, 1065353216, 1065353216, 1065353216,
                                      1065353216, 1065353216, 0,          1065353216, 1065353216,
                                      0,          0,          1065353216, 1065353216};
    for (int index = 0; index < at<int>(owner, 68); ++index) {
        void *glow = offset(at<void *>(owner, 744), Dword(index) * 248);
        char section[256];
        std::sprintf(section, "lightglow%d", index);
        at<float>(glow, 4) = 1;
        ini::get_float(reader, section, "glow_sizex", &at<float>(glow, 12));
        ini::get_float(reader, section, "glow_sizey", &at<float>(glow, 16));
        ini::get_float(reader, section, "glow_flashfreq", &at<float>(glow, 0));
        int fade = 0;
        ini::get_int(reader, section, "glow_fadeonangle", &fade);
        if (fade)
            at<Byte>(glow, 228) = 1;
        int red = 0, green = 0, blue = 0, alpha = 0;
        ini::get_int(reader, section, "glow_red", &red);
        ini::get_int(reader, section, "glow_green", &green);
        ini::get_int(reader, section, "glow_blue", &blue);
        ini::get_int(reader, section, "glow_alpha", &alpha);
        at<Byte>(glow, 224) = Byte(red);
        at<Byte>(glow, 225) = Byte(green);
        at<Byte>(glow, 226) = Byte(blue);
        at<Byte>(glow, 227) = Byte(alpha);
        at<float>(glow, 232) =
            float(static_cast<long double>(alpha) * 0.0039215688593685626983642578125f);
        float y = float(-(X(at<float>(glow, 16)) * .5f)),
              x = float(-(X(at<float>(glow, 12)) * .5f));
        at<Dword>(glow, 212) = (Dword(Byte(alpha)) << 24) | (Dword(Byte(red)) << 16) |
                               (Dword(Byte(green)) << 8) | Byte(blue);
        for (unsigned vertex = 0; vertex < 6; ++vertex) {
            at<float>(glow, 20 + vertex * 16) = y;
            at<Dword>(glow, 24 + vertex * 16) = 0;
            at<float>(glow, 28 + vertex * 16) = x;
            at<float>(glow, 32 + vertex * 16) = 1;
            std::memcpy(offset(glow, 116 + vertex * 16), coordinates + vertex * 4, 16);
        }
        for (unsigned field : {44u, 60u})
            at<float>(glow, field) = float(X(at<float>(glow, 16)) + at<float>(glow, field));
        for (unsigned field : {52u, 68u})
            at<float>(glow, field) = float(X(at<float>(glow, field)) + at<float>(glow, 12));
        char shape[32]{};
        ini::get_string(reader, section, "glow_shape", shape, 31);
        Dword *image =
            runtime::particles::pending::find_fsh_image(at<Dword *>(owner, 764), shape);
        auto *texture = static_cast<Dword *>(
            lifecycle::allocate(48, "VehicleLightGlow TAR", lifecycle::cars_pool));
        if (texture) {
            try {
                graphics::tar::construct_shape(texture, image);
            } catch (...) {
                gui::application_release(texture);
                throw;
            }
        }
        const auto ticks = static_cast<long double>(
            std::int32_t(graphics::unresolved::camera_state));
        at<Dword *>(glow, 220) = texture;
        at<Byte>(glow, 8) = 0;
        at<float>(glow, 216) = float(ticks * 0.01666666753590106964111328125f);
    }
}
// The return is incidental; all production callers discard it.
void load_light_channel(void *owner, ini::Reader *reader, Dword channel) {
    constexpr const char *sections[] = {"brakelights", "coplights", "headlights", "reverselights"};
    constexpr float directions[4][3] = {{0, 0, -1}, {0, 0, 1}, {0, 0, 1}, {0, 0, -1}};
    const char *section = sections[channel];
    void *selected = offset(owner, 72 + channel * 168);
    ini::get_int(reader, section, "glow_num", &at<int>(selected, 20));
    if (at<Dword>(selected, 20)) {
        std::memcpy(offset(selected, 4), directions[channel], 12);
        for (int index = 1; index <= at<int>(selected, 20); ++index) {
            char bone[256], key[260];
            std::sprintf(bone, "unknown.bone");
            std::sprintf(key, "attach_bone%d", index);
            ini::get_string(reader, section, key, bone, 255);
            Dword attachment = attachment_index(bone);
            if (!attachment_enabled(at<void *>(owner, 768), attachment))
                attachment = 0;
            at<Dword>(selected, 24 + Dword(index - 1) * 4) = attachment;
            create_attachment_node(at<void *>(owner, 768), attachment);
            std::sprintf(key, "attach_glow%d", index);
            at<Dword>(selected, 52 + Dword(index - 1) * 4) = 0;
            ini::get_int(reader, section, key, &at<int>(selected, 52 + Dword(index - 1) * 4));
            at<Byte>(selected, 0) = 1;
        }
    }
}

void pending::load_lights(void *owner, const char *name, void *attachments) {
    if (!graphics::device)
        return;
    at<void *>(owner, 768) = attachments;
    at<Dword>(owner, 764) = at<Dword>(subsystem(52), 78600);
    at<const char *>(owner, 772) = name;
    char filename[260];
    std::sprintf(filename, "%s%s\\vehicle.ini", offset(subsystem(4), 2084), name);
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        load_vehicle_glows(owner, &reader);
        for (unsigned channel = 0; channel < 4; ++channel) {
            void *selected = offset(owner, 72 + channel * 168);
            at<Byte>(selected, 0) = 0;
            at<Dword>(selected, 20) = at<Dword>(selected, 16) = 0;
            at<Byte>(selected, 164) = 0;
            load_light_channel(owner, &reader, channel);
        }
        auto *state = static_cast<Dword *>(owner);
        graphics::geoprim::shading(state, 1);
        graphics::geoprim::texture_enable(state, 1);
        graphics::geoprim::alpha_test(state, 0);
        graphics::geoprim::alpha_blend_mode(state, 2);
        graphics::geoprim::transparency(state, 1);
        graphics::geoprim::culling(state, 0);
        graphics::geoprim::depth_method(state, 4);
        at<Byte>(owner, 748) = 1;
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}
} // namespace nfs::game::world::vehicle_render
