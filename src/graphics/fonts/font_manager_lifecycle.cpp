#include "nfs/graphics/fonts/font_faces.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include <cstdio>
namespace nfs::fonts::faces {}
namespace nfs::lifecycle::table_entries {
namespace f = fonts::faces;
namespace objects = construction::objects;
namespace {
void release_face(f::Face *face) {
    if (face->patchwork) {
        fonts::patchwork::destroy(face->patchwork);
        gui::application_release(face->patchwork);
        face->patchwork = nullptr;
        face->font = nullptr;
    } else if (face->font) {
        fonts::release_font(face->font);
        face->font = nullptr;
    }
    if (face->file) {
        kernel::memory_free(face->file);
        face->file = nullptr;
    }
}
} // namespace

void release_fonts_when(fonts::faces::Manager *manager, int when) {
    for (int i = 0; i < manager->count; ++i)
        if (manager->faces[i].when == when)
            release_face(manager->faces + i);
}

void __fastcall initialize_fonts(System *object, void *) {
    if (!graphics::device)
        return;
    auto *manager = reinterpret_cast<f::Manager *>(object);
    char path[260];
    std::sprintf(path, "%sfonts.ini",
                 reinterpret_cast<const char *>(lifecycle::manager->slots[1]) + 2344);
    ini::Reader reader;
    ini::reader_construct(&reader, path, 0);
    try {
        int count = 0;
        for (int i = 0; i < 10; ++i) {
            char filename[260], section[32];
            filename[0] = 0;
            std::sprintf(section, "font%d", i);
            ini::get_string(&reader, section, "fontfile", filename, 259);
            if (filename[0])
                ++count;
        }
        manager->count = count;
        if (count) {
            auto *cookie = static_cast<Dword *>(files::archives::allocate_entry(
                count * sizeof(f::Face) + 4, "font faces", ui_pool));
            f::Face *faces = nullptr;
            if (cookie) {
                *cookie = count;
                faces = reinterpret_cast<f::Face *>(cookie + 1);
                construction::construct_array(faces, sizeof(f::Face), count, f::construct,
                                              f::destroy);
            }
            manager->faces = faces;
            if (faces) {
                f::read_settings(manager, &reader);
                for (int i = 0; i < manager->count; ++i)
                    if (faces[i].when == 0)
                        f::load(faces + i);
            }
        }
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}

void __fastcall shutdown_fonts(System *object, void *) {
    auto *manager = reinterpret_cast<f::Manager *>(object);
    for (int i = 0; i < manager->count; ++i)
        if (manager->faces[i].when == 0)
            release_face(manager->faces + i);
    f::Face *faces = manager->faces;
    manager->count = 0;
    if (faces) {
        auto *cookie = reinterpret_cast<Dword *>(faces) - 1;
        objects::destroy_array(faces, sizeof(f::Face), *cookie, f::destroy);
        kernel::memory_free(cookie);
    }
    manager->faces = nullptr;
}

void __fastcall enter_fonts_frontend(System *object, void *) {
    auto *manager = reinterpret_cast<f::Manager *>(object);
    for (int i = 0; i < manager->count; ++i)
        if (manager->faces[i].when != 2)
            f::load(manager->faces + i);
}

void __fastcall leave_fonts_frontend(System *object, void *) {
    auto *manager = reinterpret_cast<f::Manager *>(object);
    for (int i = 0; i < manager->count; ++i)
        if (manager->faces[i].when == 1)
            release_face(manager->faces + i);
}

void __fastcall load_fonts_level(System *object, void *) {
    auto *manager = reinterpret_cast<f::Manager *>(object);
    for (int i = 0; i < manager->count; ++i)
        if (manager->faces[i].when != 1)
            f::load(manager->faces + i);
}

void __fastcall unload_fonts_level(System *object, void *) {
    auto *manager = reinterpret_cast<f::Manager *>(object);
    for (int i = 0; i < manager->count; ++i)
        if (manager->faces[i].when == 2)
            release_face(manager->faces + i);
}
} // namespace nfs::lifecycle::table_entries
