#include "vehicle_renderable_resources.hpp"
#include "vehicle_shader_parameters.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/graphics/textures/shape_conversion.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::game::runtime::particles::pending {
Dword *find_fsh_image(Dword *, const char *);
void fsh_directory_name(Dword *, int, Dword *);
} // namespace nfs::game::runtime::particles::pending
namespace nfs::game::world::vehicle_render {
// Complete RET; loader has no stack arguments.
void register_model_runtime_textures(eagl::Loader *) {
}

void *load_body_model(void *resource, const char *name, Dword variation, Byte reload) {
    char symbol[256];
    if (!variation)
        std::sprintf(symbol, "%s", name);
    else
        std::sprintf(symbol, "%s%s%d", name, ".variation", variation);
    auto *images = at<Dword *>(resource, 0);
    if (images) {
        register_model_runtime_textures(reinterpret_cast<eagl::Loader *>(images));
        world::pending::register_level_texture_symbols(images);
    }
    if (at<void *>(resource, 4) && reload) {
        auto *loader = static_cast<eagl::Loader *>(
            lifecycle::allocate(28, "Vehicle Dynamic Loader", lifecycle::cars_pool));
        if (loader) {
            try {
                eagl::loader_construct(loader, at<eagl::ElfHeader *>(resource, 4),
                                       at<Dword>(resource, 12), graphics::symbols::resolve_module);
            } catch (...) {
                gui::application_release(loader);
                throw;
            }
        }
        at<eagl::Loader *>(resource, 16) = loader;
    }
    if (images)
        world::pending::unregister_level_texture_symbols(images);
    if (auto *loader = at<eagl::Loader *>(resource, 16)) {
        eagl::SymbolView view;
        return reinterpret_cast<void *>(
            track_render::find_model(&view, loader, symbol, 0)->address);
    }
    return nullptr;
}

void bind_model_states(void *model, Dword *state, Dword blend) {
    Dword *property = track_render::first_model_property(model, "State");
    if (property[2]) {
        graphics::geoprim::assign(state, reinterpret_cast<Dword *>(property[3]));
        graphics::geoprim::transparency(state, 1);
        graphics::geoprim::alpha_test(state, 0);
        graphics::geoprim::texture_enable(state, 1);
        graphics::geoprim::alpha_blend_mode(state, blend);
        graphics::geoprim::shading(state, 1);
        graphics::geoprim::depth_method(state, 4);
        for (int index = 0; index < int(property[2]); ++index)
            graphics::geoprim::assign(reinterpret_cast<Dword *>(property[3]) + index * 17, state);
    }
}

void set_model_layers(void *model, Dword enabled) {
    for (int index = 0; index < at<int>(model, 156); ++index) {
        const char *name = index < 0 || index >= at<int>(model, 156)
                               ? nullptr
                               : at<const char **>(model, 160)[index];
        auto *layer = track_render::find_render_layer(model, name);
        at<std::uint16_t>(layer, 2) = enabled ? 1 : 0;
    }
}

void set_vehicle_computation(void *model, Dword mode) {
    if (!graphics::device)
        return;
    Dword *begin =
        track_render::first_model_property(model, "ComputationIndex::vehicleComputationIndex");
    Dword *end =
        track_render::last_model_property(model, "ComputationIndex::vehicleComputationIndex");
    for (Dword *property = begin; property != end; property += 4)
        if (int(property[2]) > 0)
            *reinterpret_cast<std::uint16_t *>(property[3]) = std::uint16_t(mode);
}
// Unlike the directory-only lookup, this retains shape-name writes.
Dword *find_named_shape(Dword *image, const char *name) {
    for (int index = 0; index < int(image[2]); ++index) {
        auto *shape =
            reinterpret_cast<Dword *>(reinterpret_cast<Dword>(image) + image[5 + index * 2]);
        char *shape_name = const_cast<char *>(graphics::textures::shape::shape_name(shape));
        char directory[8];
        if (!shape_name || std::strlen(shape_name) > 4) {
            runtime::particles::pending::fsh_directory_name(image, index,
                                                            reinterpret_cast<Dword *>(directory));
            for (char *last = directory + 3; last >= directory && *last == ' '; --last)
                *last = 0;
            shape_name = directory;
        }
        shape_name[4] = 0;
        if (!std::strcmp(name, shape_name))
            return shape;
    }
    return nullptr;
}

void bind_model_fsh(void *model, Dword *image) {
    auto *cursor = at<Dword *>(model, 188);
    while (*cursor) {
        const char *name = reinterpret_cast<const char *>(*cursor++);
        Dword *shape = image ? find_named_shape(image, name) : nullptr;
        Dword count = *cursor++;
        while (count--) {
            auto *texture = reinterpret_cast<Dword *>(*cursor++ + at<Dword>(model, 192) * 48);
            if (shape || !image)
                graphics::tar::replace_image(texture, shape);
        }
    }
}

void construct_texture_binding(Dword *binding) {
    binding[0] = Dword(-1);
    binding[1] = binding[2] = 0;
}

void find_model_texture(void *model, Dword *result, const char *name) {
    Dword binding[3];
    construct_texture_binding(binding);
    if (name) {
        auto *cursor = at<Dword *>(model, 188);
        while (*cursor) {
            if (!std::strcmp(reinterpret_cast<const char *>(*cursor), name)) {
                result[0] = at<Dword>(model, 192);
                result[1] = cursor[1];
                result[2] = reinterpret_cast<Dword>(cursor + 2);
                return;
            }
            cursor += cursor[1] + 2;
        }
    }
    std::memcpy(result, binding, 12);
}

void replace_model_texture(void *model, Dword *binding, Dword *shape) {
    for (int index = 0; index < int(binding[1]); ++index)
        graphics::tar::replace_image(
            reinterpret_cast<Dword *>(reinterpret_cast<Dword *>(binding[2])[index] +
                                     at<Dword>(model, 192) * 48),
            shape);
}

void bind_model_shape(void *model, const char *name, Dword *shape) {
    Dword binding[3];
    find_model_texture(model, binding, name);
    replace_model_texture(model, binding, shape);
}

void read_geometry_configuration(void *owner, void *model, const char *resource) {
    if (!graphics::device)
        return;
    char filename[260], section[268];
    std::sprintf(filename, "%s%s\\geomdata.ini", offset(subsystem(4), 2084), resource);
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        ini::get_int(&reader, "geomdata", "geomnum", &at<int>(owner, 1848));
        at<Byte>(owner, 1852) = 1;
        at<void *>(owner, 1856) = model;
        std::sprintf(filename, "%s%s\\geomdata.ini", offset(subsystem(4), 2084), resource);
        for (int index = 0; index < at<int>(at<void *>(owner, 1856), 156); ++index) {
            void *geometry = offset(owner, Dword(index) * 24);
            at<Dword>(geometry, 8) = 0;
            at<float>(geometry, 4) = 100;
            at<Dword>(geometry, 0) = 0;
            at<Dword>(geometry, 16) = Dword(-1);
            at<Byte>(geometry, 22) = 0;
            void *current = at<void *>(owner, 1856);
            const char *name = index < 0 || index >= at<int>(current, 156)
                                   ? nullptr
                                   : at<const char **>(current, 160)[index];
            std::sprintf(section, "%s", name);
            at<Dword *>(geometry, 0) = track_render::find_render_layer(model, section);
            ini::get_float(&reader, section, "lodoff", &at<float>(geometry, 8));
            ini::get_float(&reader, section, "lodon", &at<float>(geometry, 4));
            ini::get_int(&reader, section, "damagezoneid", &at<int>(geometry, 16));
            int damaged = 0;
            ini::get_int(&reader, section, "damageversion", &damaged);
            if (damaged)
                at<Byte>(geometry, 22) = 1;
            at<Dword>(geometry, 12) = 1;
            at<Byte>(geometry, 23) = 0;
            if (std::strstr(section, "ALPHA_TRANS"))
                at<Dword>(geometry, 12) = 2;
            if (std::strstr(section, "ALPHA_ADD"))
                at<Dword>(geometry, 12) = 4;
            if (std::strstr(section, "GEOM_STEERINGWHEEL"))
                at<Byte>(geometry, 23) = 1;
            at<Byte>(geometry, 21) = 0;
            at<Byte>(geometry, 20) = std::strstr(section, "ADDON_") != nullptr;
            if (at<Byte>(geometry, 20))
                at<Dword>(geometry, 12) |= 8;
        }
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}

void bind_body_configuration(void *body, const char *name) {
    read_geometry_configuration(offset(body, 36), at<void *>(body, 28), name);
}

void colour_components(int red, int green, int blue, int alpha, float *output) {
    const auto format = kernel::numeric::current_format();
    const int channels[] = {red, green, blue, alpha};
    for (unsigned i = 0; i < 4; ++i)
        output[i] = float(kernel::numeric::Extended::integer(channels[i], format) * 0.0039215688593685626983642578125f);
}

void bind_body_skin(void *body, Dword catalog, Dword skin) {
    void *colour = catalogue_skin(offset(subsystem(52), 4), catalog, skin);
    colour_components(at<int>(colour, 20), at<int>(colour, 24), at<int>(colour, 28), 255,
                      reinterpret_cast<float *>(offset(body, 2092)));
    void *entry = offset(subsystem(52), 8 + catalog * 1228);
    for (unsigned i = 0; i < 3; ++i)
        at<Dword>(body, 4 + i * 4) = at<Dword>(colour, 32 + i * 4);
    at<Dword>(body, 2108) = at<Dword>(colour, 44);
    at<Dword>(body, 2068) = at<Dword>(entry, 1220);
    at<Dword>(body, 2064) = at<Dword>(entry, 1224);
}

void bind_shadow_extents(void *body, Dword catalog, const float *extents) {
    float scale = at<float>(subsystem(52), 8 + catalog * 1228 + 1216);
    const auto format = kernel::numeric::current_format();
    at<float>(body, 2056) = float(kernel::numeric::Extended(scale, format) * extents[0]);
    at<float>(body, 2060) = float(kernel::numeric::Extended(extents[2], format) * scale);
}

void enable_geometry_addons(void *geometry) {
    for (int index = 0; index < at<int>(geometry, 1848); ++index)
        at<Byte>(geometry, Dword(index) * 24 + 21) = 1;
}

void enable_body_addons(void *body) {
    enable_geometry_addons(offset(body, 36));
}
} // namespace nfs::game::world::vehicle_render
namespace nfs::game::world::vehicle_render::pending {
// Six stack arguments, RET24.
void __fastcall load_body_models(void *body, void *, Dword *images, Dword car_loader,
                                 Dword shadow_loader, Dword skin, Dword, Dword variation) {
    at<Dword>(body, 2144) = skin;
    at<Dword>(body, 2148) = at<Dword>(body, 2164) = at<Dword>(body, 2168) = 0;
    at<Dword>(body, 2160) = car_loader;
    at<Dword>(body, 2180) = shadow_loader;
    char filename[256];
    std::sprintf(filename, "%s%s", "car", ".fsh");
    at<Dword>(body, 2348) = images[0];
    std::sprintf(filename, "%s%s", "damage", ".fsh");
    at<Dword>(body, 2360) = images[1];
    if (images[1]) {
        auto *image = reinterpret_cast<Dword *>(images[1]);
        Dword displacement = image[5];
        auto *texture = static_cast<Dword *>(
            lifecycle::allocate(48, "VEHICLE DAMAGE TAR", lifecycle::cars_pool));
        if (texture) {
            try {
                graphics::tar::construct_shape(
                    texture,
                    reinterpret_cast<Dword *>(reinterpret_cast<Dword>(image) + displacement));
            } catch (...) {
                gui::application_release(texture);
                throw;
            }
        }
        at<Dword *>(body, 2340) = texture;
    }
    auto *image = at<Dword *>(body, 2348);
    Dword *high_wheels = nullptr, *lod_wheels = nullptr;
    if (image && at<Dword>(body, 2128) == 16)
        high_wheels = runtime::particles::pending::find_fsh_image(image, "wlhp");
    if (image)
        lod_wheels = runtime::particles::pending::find_fsh_image(image, "wlod");
    void *model = load_body_model(offset(body, 2144), "car", variation, 0);
    at<void *>(body, 28) = model;
    if (skin)
        bind_model_fsh(model, reinterpret_cast<Dword *>(skin));
    if (high_wheels)
        bind_model_shape(model, "wl00", high_wheels);
    else if (at<Dword *>(body, 2348))
        bind_model_fsh(model, at<Dword *>(body, 2348));
    if (lod_wheels)
        bind_model_shape(model, "wlod", lod_wheels);
    if (model) {
        auto *state = reinterpret_cast<Dword *>(offset(body, 2204));
        bind_model_states(model, state, 1);
        graphics::geoprim::shading(state, 1);
        set_model_layers(model, 0);
    }
    if (at<Dword>(body, 2180)) {
        model = load_body_model(offset(body, 2164), "shadow", 0, 0);
        at<void *>(body, 32) = model;
        if (model) {
            auto *state = reinterpret_cast<Dword *>(offset(body, 2272));
            graphics::geoprim::shading(state, 1);
            bind_model_states(model, state, 1);
        }
    }
    if (!at<Byte>(subsystem(52), 81296)) {
        Dword *environment = shader_parameters.static_environment_texture;
        if (at<Dword>(subsystem(52), 81272) == 1 || runtime::physics::split_screen())
            graphics::tar::replace_shared(shader_parameters.environment_texture + 10, environment);
        else if (at<Dword>(subsystem(52), 81272) == 2 && at<Byte>(subsystem(32), 4))
            graphics::tar::replace_shared(shader_parameters.environment_texture + 10,
                                          at<Dword *>(subsystem(32), 412));
    }
    if (shader_parameters.damage_texture && at<Dword *>(body, 2340) &&
        at<Dword>(subsystem(52), 81280) == 1)
        graphics::tar::replace_shared(shader_parameters.damage_texture + 10,
                                      at<Dword *>(body, 2340));
    Dword mode = at<Dword>(subsystem(52), 81288);
    at<Dword>(body, 2452) = mode == 1 ? 1 : mode == 2 ? 2 : 0;
    set_vehicle_computation(at<void *>(body, 28), at<Dword>(body, 2452));
}
} // namespace nfs::game::world::vehicle_render::pending
