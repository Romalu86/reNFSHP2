#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "gui_images.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace nfs::game::runtime::particles::pending {
Dword *find_fsh_image(Dword *, const char *);
void fsh_directory_name(Dword *, int, Dword *);
} // namespace nfs::game::runtime::particles::pending
namespace nfs::gui::engine::images {
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
namespace particle = game::runtime::particles;
namespace scratch = particle::lifecycle;
static Dword texture_serial{};
Engine *resource_engine() {
    void *frontend = game::runtime::system(96);
    Engine *engine = at<Engine *>(frontend, 4);
    return engine ? engine : at<Engine *>(frontend, 8);
}
namespace {
void release_file(Texture *texture) {
    if (texture->file) {
        kernel::memory_free(texture->file);
        texture->file = nullptr;
    }
    if (texture->raster) {
        graphics::tar::destroy(texture->raster);
        eagl::deallocator(texture->raster, 48);
        texture->raster = nullptr;
    }
    texture->format = texture->frame_count = 0;
}
} // namespace
namespace texture_pending {
Byte conversion_required(graphics::TextureReader *, Dword *);
void copy_scratch(scratch::TextureScratch *, const void *, Dword);
} // namespace texture_pending

Texture *construct_texture(Texture *texture, const char *filename, const char *directory,
                           const char *name) {
    texture->methods = gui::base_methods;
    kernel::lifecycle::Cleanup base_cleanup([=] { texture->methods = gui::base_methods; });
    texture->file = texture->raster = texture->atlas = nullptr;
    texture->frame_count = texture->format = 0;
    texture->references = 0;
    texture->handle = texture_serial += 4;
    string_construct(&texture->directory);
    kernel::lifecycle::Cleanup directory_base_cleanup([=] { texture->directory.methods = gui::base_methods; });
    string_assign(&texture->directory, directory);
    directory_base_cleanup.dismiss();
    kernel::lifecycle::Cleanup directory_cleanup([=] { string_destruct(&texture->directory); });
    string_construct(&texture->name);
    kernel::lifecycle::Cleanup name_base_cleanup([=] { texture->name.methods = gui::base_methods; });
    string_assign(&texture->name, name);
    name_base_cleanup.dismiss();
    kernel::lifecycle::Cleanup name_cleanup([=] { string_destruct(&texture->name); });
    const auto complete = [&] {
        name_cleanup.dismiss();
        directory_cleanup.dismiss();
        base_cleanup.dismiss();
        return texture;
    };
    texture->methods = texture_methods;
    if (filename && *filename) {
        char archive[264];
        std::sprintf(archive, "%s%s", bytes(game::runtime::system(4)) + 2344,
                     "gui\\guiart.viv");
        const Dword flags = lifecycle::ui_pool | 1280;
        texture->file =
            static_cast<Dword *>(files::archives::load(archive, filename, flags, 0));
        if (texture->file) {
            if (texture_pending::conversion_required(&graphics::textures::database,
                                                     texture->file)) {
                scratch::TextureScratch buffer;
                scratch::acquire_texture_scratch(&buffer, &graphics::textures::database);
                try {
                    texture_pending::copy_scratch(&buffer, texture->file,
                                                  kernel::memory_size(texture->file));
                    kernel::memory_free(texture->file);
                    texture->file = nullptr;
                    texture->file = static_cast<Dword *>(scratch::pending::convert_fsh_image(
                        &graphics::textures::database,
                        scratch::texture_scratch_data(&buffer), flags));
                    if (!texture->file)
                        release_file(texture);
                } catch (...) {
                    scratch::release_texture_scratch(&buffer);
                    throw;
                }
                scratch::release_texture_scratch(&buffer);
                if (!texture->file)
                    return complete();
            }
            Dword *file = texture->file;
            Dword *first = reinterpret_cast<Dword *>(bytes(file) + file[5]);
            const short width = at<short>(first, 4), height = at<short>(first, 6);
            texture->format = width == height && height == 1 ? 2 : 1;
            texture->frame_count = int(file[2]);
            if (texture->format && texture->frame_count) {
                if (texture->format != 2)
                    return complete();
                texture->atlas = particle::pending::find_fsh_image(file, "MAIN");
                if (texture->atlas) {
                    auto *raster = static_cast<Dword *>(
                        lifecycle::allocate(48, "feTex", lifecycle::ui_pool));
                    kernel::lifecycle::Cleanup raster_cleanup([=] {
                        game::runtime::track::pending::release_named(
                            raster, "feTex", lifecycle::ui_pool);
                    });
                    Dword *constructed = raster ? graphics::tar::construct_shape(raster, texture->atlas) : nullptr;
                    raster_cleanup.dismiss();
                    texture->raster = constructed;
                    --texture->frame_count;
                    return complete();
                }
            }
            release_file(texture);
        }
    }
    return complete();
}

void destroy_texture(Texture *texture) {
    texture->methods = texture_methods;
    kernel::lifecycle::Cleanup base_cleanup([=] { texture->methods = gui::base_methods; });
    kernel::lifecycle::Cleanup directory_cleanup([=] { string_destruct(&texture->directory); });
    kernel::lifecycle::Cleanup name_cleanup([=] { string_destruct(&texture->name); });
    release_file(texture);
}

static Texture *__fastcall delete_texture(Texture *texture, void *, unsigned flags) {
    destroy_texture(texture);
    if (flags & 1)
        gui::release(texture);
    return texture;
}
const Destroy texture_methods[] = {reinterpret_cast<Destroy>(delete_texture)};

String *frame_name(Texture *texture, String *out, int index) {
    if (!texture->file)
        return string_construct_copy(out,
                                     &lifecycle::construction::unresolved::shared_string);
    char name[256];
    particle::pending::fsh_directory_name(texture->file, index, reinterpret_cast<Dword *>(name));
    // The entry passes the four directory bytes directly; the stack tail is an open ABI case.
    string_construct(out);
    kernel::lifecycle::Cleanup base_cleanup([=] { out->methods = gui::base_methods; });
    string_assign(out, name);
    base_cleanup.dismiss();
    return out;
}

Image *frame(Texture *texture, const String *name, Dword indexed) {
    if (!name->length || !texture->file)
        return nullptr;
    char short_name[5]{};
    for (int index = 0; index < 4; ++index)
        short_name[index] = char(pending::character_at(name, index));
    for (int index = 1; index < 4; ++index)
        if (!short_name[index])
            short_name[index] = ' ';
    if (texture->format == 1) {
        if (Dword *shape = particle::pending::find_fsh_image(texture->file, short_name)) {
            auto *image = static_cast<ShapeImage *>(allocate(sizeof(ShapeImage)));
            if (image) {
                String key;
                string_construct(&key);
                try {
                    string_assign(&key, indexed ? nullptr : short_name);
                    construct_shape(image, shape, &key, texture->handle);
                } catch (...) {
                    string_destruct(&key);
                    throw;
                }
                string_destruct(&key);
            }
            if (image)
                container_append(&resource_engine()->images, image, dispose_object);
            return reinterpret_cast<Image *>(image);
        }
    } else if (texture->format == 2 && texture->atlas) {
        for (;;) {
            if (Dword *shape =
                    particle::pending::find_fsh_image(texture->file, short_name)) {
                auto *image = static_cast<AtlasImage *>(allocate(sizeof(AtlasImage)));
                if (image) {
                    String key;
                    string_construct(&key);
                    try {
                        string_assign(&key, indexed ? nullptr : short_name);
                        const int coordinates = int(shape[3]);
                        construct_atlas(image, texture->atlas, texture->raster, &key,
                                        int(Dword(coordinates) << 20) >> 20,
                                        int(Dword(coordinates) << 4) >> 20, at<short>(shape, 8),
                                        at<short>(shape, 10), texture->handle);
                    } catch (...) {
                        string_destruct(&key);
                        throw;
                    }
                    string_destruct(&key);
                }
                if (image)
                    container_append(&resource_engine()->images, image, dispose_object);
                return reinterpret_cast<Image *>(image);
            }
            const auto length = std::strlen(short_name);
            if (!length || short_name[length - 1] != ' ')
                break;
            short_name[length - 1] = 0;
        }
    }
    return nullptr;
}

Image *frame_at(Texture *texture, int index, Dword indexed) {
    String name;
    frame_name(texture, &name, index);
    try {
        Image *image = frame(texture, &name, indexed);
        string_destruct(&name);
        return image;
    } catch (...) {
        string_destruct(&name);
        throw;
    }
}

Dword rebind(Texture *texture, Image *image, int index) {
    if (!texture->file || index < 0 || index > texture->frame_count)
        return 0;
    Dword *shape =
        reinterpret_cast<Dword *>(bytes(texture->file) + texture->file[5 + Dword(index) * 2]);
    if (!shape || (texture->format != 1 && texture->format != 2))
        return 0;
    char name[260];
    particle::pending::fsh_directory_name(texture->file, index, reinterpret_cast<Dword *>(name));
    name[4] = 0;
    const int packed = int(shape[3]);
    return invoke<Dword>(image, 4, texture->format == 1 ? shape : texture->atlas,
                        texture->format == 1 ? nullptr : texture->raster, int(Dword(packed) << 20) >> 20,
                        int(Dword(packed) << 4) >> 20, at<short>(shape, 8), at<short>(shape, 10),
                        texture->handle);
}
} // namespace nfs::gui::engine::images
