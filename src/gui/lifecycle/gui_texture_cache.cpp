#include "gui_images.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::game::runtime::particles::pending {
void fsh_directory_name(Dword *, int, Dword *);
}
namespace nfs::gui::engine::images {
namespace {
struct LocalString : String {
    LocalString() {
        string_construct(this);
    }
    ~LocalString() {
        string_destruct(this);
    }
};
} // namespace

Texture *find_texture(Engine *engine, const String *name) {
    LocalString directory, filename;
    const int separator = find_separator(name, ":");
    string_substring(name, 0, separator, &directory);
    string_substring(name, separator + 1, name->length, &filename);
    uppercase(&directory);
    uppercase(&filename);
    if (string_data(&filename)) {
        const Dword count = engine->textures.count;
        for (Dword index = 0; index < count; ++index)
            if (auto *texture =
                    static_cast<Texture *>(events::container_value(&engine->textures, index)))
                if (string_equal(&texture->name, &filename) &&
                    string_equal(&texture->directory, &directory))
                    return texture;
    }
    return nullptr;
}

Image *find_image(Engine *engine, Dword texture, const String *name) {
    if (texture && string_data(name)) {
        const Dword count = engine->images.count;
        for (Dword index = 0; index < count; ++index)
            if (auto *image = static_cast<Image *>(events::container_value(&engine->images, index)))
                if (image->texture == texture && string_equal(&image->name, name))
                    return image;
    }
    return nullptr;
}
} // namespace nfs::gui::engine::images
namespace nfs::gui::engine::pending {
using images::Image;
using images::Texture;
namespace {
struct LocalString : String {
    LocalString() {
        string_construct(this);
    }
    ~LocalString() {
        string_destruct(this);
    }
};
Texture *texture_by_handle(Engine *engine, Dword handle) {
    const auto compare = [](const void *record, const void *key) {
        const Dword handle = static_cast<const Texture *>(record)->handle;
        const Dword wanted = reinterpret_cast<Dword>(key);
        return handle < wanted ? -1 : wanted < handle;
    };
    return static_cast<Texture *>(
        container_find(&engine->textures, compare, reinterpret_cast<void *>(handle)));
}
Image *image_by_handle(Engine *engine, Dword handle) {
    const auto compare = [](const void *record, const void *key) {
        const Dword handle = static_cast<const Image *>(record)->handle;
        const Dword wanted = reinterpret_cast<Dword>(key);
        return handle < wanted ? -1 : wanted < handle;
    };
    return static_cast<Image *>(
        container_find(&engine->images, compare, reinterpret_cast<void *>(handle)));
}
void image_path(char *path, const String *name) {
    std::sprintf(path, "%s%s%s%s", game::runtime::bytes(game::runtime::system(4)) + 2604, "pc\\",
                 string_ascii(name), ".fsh");
}
const String *empty_name() {
    return &lifecycle::construction::unresolved::shared_string;
}
} // namespace

Dword __fastcall load_texture(Engine *engine, void *, const String *name) {
    if (!engine->context || !string_data(name))
        return 0;
    if (Texture *texture = images::find_texture(engine, name)) {
        ++texture->references;
        return texture->handle;
    }
    LocalString directory, filename;
    const int separator = images::find_separator(name, ":");
    string_substring(name, 0, separator, &directory);
    string_substring(name, separator + 1, name->length, &filename);
    uppercase(&directory);
    uppercase(&filename);
    if (!string_data(&filename) || !string_data(&directory))
        return 0;
    char path[260]{};
    image_path(path, &filename);
    auto *texture = static_cast<Texture *>(allocate(sizeof(Texture)));
    if (texture)
        images::construct_texture(texture, path, string_ascii(&directory), string_ascii(&filename));
    ++texture->references;
    container_append(&engine->textures, texture, dispose_object);
    return texture->handle;
}

Dword __fastcall load_image(Engine *engine, void *, const String *name) {
    if (!engine->context)
        return 0;
    LocalString directory, filename, frame_name;
    if (!string_data(name))
        return 0;
    const int open = images::find_separator(name, "["), close = images::find_separator(name, "]"),
              separator = images::find_separator(name, ":");
    string_substring(name, open + 1, separator - open - 1, &directory);
    string_substring(name, separator + 1, close - separator - 1, &filename);
    string_substring(name, close + 1, name->length - close, &frame_name);
    images::trim_left(&directory, 32);
    images::trim_right(&directory, 32);
    uppercase(&directory);
    images::trim_left(&filename, 32);
    images::trim_right(&filename, 32);
    uppercase(&filename);
    images::trim_left(&frame_name, 32);
    images::trim_right(&frame_name, 32);
    if (!string_data(&directory) || !string_data(&filename))
        return 0;
    char path[260];
    image_path(path, &filename);
    Texture *texture = nullptr;
    const Dword count = engine->textures.count;
    for (Dword index = 0; index < count; ++index)
        if (auto *candidate =
                static_cast<Texture *>(events::container_value(&engine->textures, index)))
            if (string_equal(&candidate->directory, &directory) &&
                string_equal(&candidate->name, &filename)) {
                texture = candidate;
                if (Image *image = images::find_image(engine, texture->handle, &frame_name)) {
                    ++image->references;
                    return image->handle;
                }
                break;
            }
    const bool created = !texture;
    if (created) {
        texture = static_cast<Texture *>(allocate(sizeof(Texture)));
        if (texture)
            images::construct_texture(texture, path, string_ascii(&directory),
                                      string_ascii(&filename));
        container_append(&engine->textures, texture, dispose_object);
        if (!texture)
            return 0;
    }
    if (Image *image = images::frame(texture, &frame_name, 0))
        return image->handle;
    if (created)
        container_remove(&engine->textures, texture);
    return 0;
}

String *__fastcall texture_name(Engine *engine, void *, String *out, Dword handle) {
    if (Texture *texture = handle ? texture_by_handle(engine, handle) : nullptr) {
        String prefix;
        append_ascii_value(&prefix, &texture->directory, ":");
        try {
            concatenate(out, &prefix, &texture->name);
        } catch (...) {
            string_destruct(&prefix);
            throw;
        }
        string_destruct(&prefix);
        return out;
    }
    return string_construct_copy(out, empty_name());
}

static String *compose_image_name(String *out, const String *directory, const String *texture,
                                  const String *frame) {
    if (!string_data(texture) || !string_data(frame))
        return string_construct_copy(out, empty_name());
    String prefix;
    prepend_ascii_value(&prefix, "[", directory);
    try {
        String separator;
        append_ascii_value(&separator, &prefix, ":");
        try {
            String texture_name;
            concatenate(&texture_name, &separator, texture);
            try {
                String close;
                append_ascii_value(&close, &texture_name, "]");
                try {
                    String space;
                    append_ascii_value(&space, &close, " ");
                    try {
                        concatenate(out, &space, frame);
                    } catch (...) {
                        string_destruct(&space);
                        throw;
                    }
                    string_destruct(&space);
                } catch (...) {
                    string_destruct(&close);
                    throw;
                }
                string_destruct(&close);
            } catch (...) {
                string_destruct(&texture_name);
                throw;
            }
            string_destruct(&texture_name);
        } catch (...) {
            string_destruct(&separator);
            throw;
        }
        string_destruct(&separator);
    } catch (...) {
        string_destruct(&prefix);
        throw;
    }
    string_destruct(&prefix);
    return out;
}

String *__fastcall image_name(Engine *engine, void *, String *out, Dword handle) {
    if (Image *image = handle ? image_by_handle(engine, handle) : nullptr)
        if (Texture *texture = image->texture ? texture_by_handle(engine, image->texture) : nullptr)
            return compose_image_name(out, &texture->directory, &texture->name, &image->name);
    return string_construct_copy(out, empty_name());
}

Dword __fastcall find_texture_frame(Engine *engine, void *, Dword handle, const String *name) {
    if (!handle || !string_data(name))
        return 9;
    Texture *texture = texture_by_handle(engine, handle);
    if (!texture)
        return ~Dword(0);
    char wanted[5]{};
    for (int index = 0; index < 4; ++index)
        wanted[index] = char(character_at(name, index));
    if (wanted[0]) {
        for (int index = 1; index < 4; ++index)
            if (!wanted[index])
                wanted[index] = ' ';
        if (texture->file)
            for (int index = 0; index < texture->frame_count; ++index) {
                char candidate[5]{};
                game::runtime::particles::pending::fsh_directory_name(
                    texture->file, index, reinterpret_cast<Dword *>(candidate));
                if (!std::strcmp(wanted, candidate))
                    return Dword(index);
            }
    }
    return ~Dword(0);
}

Dword __fastcall bind_image_frame(Engine *engine, void *, Dword texture_handle, Dword image_handle,
                                 int index) {
    if (!texture_handle || !image_handle)
        return 9;
    Texture *texture = texture_by_handle(engine, texture_handle);
    if (!texture)
        return 6;
    if (index < 0 || index >= texture->frame_count)
        return 3;
    Image *image = image_by_handle(engine, image_handle);
    if (!image)
        return 6;
    if (image->texture != texture->handle)
        return 9;
    return images::rebind(texture, image, index) ? 0 : 4;
}
void *texture_frame_object(void *texture, int index, Dword indexed) {
    return images::frame_at(static_cast<Texture *>(texture), index, indexed);
}
} // namespace nfs::gui::engine::pending
