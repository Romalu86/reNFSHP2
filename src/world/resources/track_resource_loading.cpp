#include "track_render_resources.hpp"
#include "../../graphics/textures/texture_fsh_convert.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/graphics/fonts/font_faces.hpp"
#include <cstdio>
#include <cstring>
#include <initializer_list>
namespace nfs::game::world::track_render {

TextureOptions *begin_texture_options(TextureOptions *options, graphics::TextureReader *reader) {
    options->reader = reader;
    reader->fields[7] = reinterpret_cast<Dword>(options);
    std::memset(options->flags, 0, 6);
    return options;
}

void end_texture_options(TextureOptions *options) {
    options->reader->fields[7] = reinterpret_cast<Dword>(options->reader) + 32;
}

eagl::Loader *link_track_module(void *image, Dword bytes, void *textures, eagl::Resolve resolve) {
    if (textures)
        pending::register_level_texture_symbols(textures);
    auto *loader = static_cast<eagl::Loader *>(
        lifecycle::allocate(28, "DynamicLoader", lifecycle::track_pool | 256));
    if (loader) {
        try {
            eagl::loader_construct(loader, static_cast<eagl::ElfHeader *>(image), bytes, resolve);
        } catch (...) {
            gui::application_release(loader);
            throw;
        }
    }
    if (textures)
        pending::unregister_level_texture_symbols(textures);
    return loader;
}

void *load_persistent_fsh(graphics::TextureReader *reader, const char *archive, const char *name) {
    if (!graphics::device)
        return nullptr;
    auto *input =
        static_cast<Dword *>(files::archives::load(archive, name, lifecycle::track_pool | 256, 0));
    if (!input)
        return nullptr;
    int available = kernel::memory_largest_free(lifecycle::track_pool);
    auto *output = static_cast<Dword *>(
        kernel::memory_allocate("persist_shape", available - 1024, lifecycle::track_pool | 1024));
    if (!output) {
        kernel::memory_free(input);
        return nullptr;
    }
    Dword *end =
        graphics::textures::convert_fsh_into(reader, input, output, Dword(available - 1024));
    kernel::memory_validate_all();
    kernel::memory_free(input);
    if (!end) {
        kernel::memory_free(output);
        return nullptr;
    }
    int used = reinterpret_cast<Dword>(end) - reinterpret_cast<Dword>(output);
    kernel::memory_resize(output, used);
    if (!used)
        files::file::save("c:\\test.fsh", output, 0);
    kernel::memory_largest_free(lifecycle::track_pool);
    return output;
}

void construct_compartment(void *owner, Dword index) {
    at<Dword>(owner, 0) = index;
    at<Dword>(owner, 68) = 0;
    at<Byte>(owner, 72) = 0;
    for (unsigned field : {76u, 160u, 164u, 4u, 8u, 24u})
        at<Dword>(owner, field) = 0;
    std::memset(offset(owner, 84), 0, 48);
    at<Dword>(owner, 60) = Dword(-1);
}
} // namespace nfs::game::world::track_render
namespace nfs::game::world::pending {

void *track_texture_resources(void *owner) {
    return at<void *>(owner, 96);
}

void load_compartment(void *owner) {
    void *textures = track_texture_resources(subsystem(36));
    char filename[260];
    std::sprintf(filename, "%s%s\\comp%02d.o", offset(subsystem(4), 524),
                 offset(subsystem(40), 140), at<Dword>(owner, 0));
    void *image = files::file::try_load(filename, lifecycle::track_pool | 1024);
    at<void *>(owner, 64) = image;
    if (textures && image) {
        eagl::Loader *loader = track_render::link_track_module(
            image, kernel::memory_size(image), textures, graphics::symbols::resolve_module);
        at<eagl::Loader *>(owner, 76) = loader;
        track_render::bind_compartment_model(owner, loader);
        eagl::detach(loader);
    }
}

void load_compartment_directory(void *owner) {
    using namespace track_render;
    char archive[260];
    std::sprintf(archive, "%s%s\\persist.viv", offset(subsystem(4), 524),
                 offset(subsystem(40), 140));
    TextureOptions options;
    begin_texture_options(&options, &graphics::textures::database);
    try {
        options.flags[2] = graphics::startup::mipmaps_enabled(&graphics::startup::settings);
        options.flags[4] = options.flags[5] = 1;
        if (at<int>(subsystem(72), 2380) == 0)
            options.flags[3] = 1;
        void *textures = load_persistent_fsh(&graphics::textures::database, archive, "track.fsh");
        at<void *>(owner, 20) = textures;
        void *image = files::archives::load(archive, "trackg.o", lifecycle::track_pool | 1024, 0);
        at<void *>(owner, 0) = image;
        if (textures && image) {
            auto *loader = link_track_module(image, kernel::memory_size(image), textures,
                                             graphics::symbols::resolve_module);
            at<eagl::Loader *>(owner, 24) = loader;
            bind_directory_model(owner, loader);
            eagl::detach(loader);
        }
        textures = load_persistent_fsh(&graphics::textures::database, archive, "sky.fsh");
        at<void *>(owner, 16) = textures;
        image = files::archives::load(archive, "skyg.o", lifecycle::track_pool | 1024, 0);
        at<void *>(owner, 4) = image;
        if (textures && image) {
            auto *loader = link_track_module(image, kernel::memory_size(image), textures,
                                             graphics::symbols::resolve_module);
            at<eagl::Loader *>(owner, 80) = loader;
            eagl::SymbolView view;
            void *model = reinterpret_cast<void *>(first_model(&view, loader)->address);
            at<void *>(owner, 84) = model;
            configure_track_states(model);
            eagl::detach(loader);
        }
        if (at<int>(subsystem(24), 76) > 0) {
            float colour[4] = {1, 1, 1, 1}, size = 2000;
            char filename[260], texture[8] = "SUNGLW";
            std::sprintf(filename, runtime::level_file("lighting.ini"));
            ini::Reader reader;
            ini::reader_construct(&reader, filename, 0);
            try {
                ini::get_float(&reader, "SunGlow", "red", colour);
                ini::get_float(&reader, "SunGlow", "green", colour + 1);
                ini::get_float(&reader, "SunGlow", "blue", colour + 2);
                ini::get_float(&reader, "SunGlow", "size", &size);
                ini::get_string(&reader, "SunGlow", "texture", texture, 8);
                textures = load_persistent_fsh(&graphics::textures::database, archive, "sun.fsh");
                at<void *>(owner, 8) = textures;
                if (textures) {
                    void *glow = lifecycle::allocate(416, "Sun LightGlow", lifecycle::level_pool);
                    if (glow) {
                        try {
                            construct_sun_glow(glow);
                        } catch (...) {
                            gui::application_release(glow);
                            throw;
                        }
                    }
                    at<void *>(owner, 88) = glow;
                    bind_sun_glow(glow,
                                  static_cast<float *>(runtime::level_marker(
                                      static_cast<Dword *>(subsystem(24)), 4, 0)),
                                  textures, size, colour);
                }
            } catch (...) {
                ini::reader_clear(&reader);
                throw;
            }
            ini::reader_clear(&reader);
        }
        at<void *>(owner, 12) =
            load_persistent_fsh(&graphics::textures::database, archive, "lightglows.fsh");
    } catch (...) {
        end_texture_options(&options);
        throw;
    }
    end_texture_options(&options);
}
} // namespace nfs::game::world::pending
