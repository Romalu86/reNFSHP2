#pragma once
#include "nfs/graphics/fonts/patchwork_font.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
namespace nfs::fonts::faces {
struct Face {
    char filename[32];
    Font *file, *font;
    patchwork::Cache *patchwork;
    kernel::Dword style_2c, style_30, style_34, style_38, style_3c;
    int when;
    kernel::Dword unknown_44;
    float size_ratio;
    Byte use_patchwork, padding[3];
};
struct Manager {
    const lifecycle::SystemMethods *methods;
    Face *faces;
    int count;
};
static_assert(sizeof(Face) == 80 && sizeof(Manager) == 12 && offsetof(Face, when) == 64);
extern kernel::Dword saved_pool;
void begin_font_pool();
void end_font_pool();
void *construct(void *);
void destroy(void *);
void load(Face *);
void set_filename(Face *, const char *);
void read_settings(Manager *, ini::Reader *);
int count_settings(ini::Reader *);
void draw(Face *, kernel::Dword *, float, float, const char *);
} // namespace nfs::fonts::faces
namespace nfs::lifecycle::table_entries {
void release_fonts_when(fonts::faces::Manager *, int);
void __fastcall initialize_fonts(System *, void *);
void __fastcall shutdown_fonts(System *, void *);
void __fastcall enter_fonts_frontend(System *, void *);
void __fastcall leave_fonts_frontend(System *, void *);
void __fastcall load_fonts_level(System *, void *);
void __fastcall unload_fonts_level(System *, void *);
} // namespace nfs::lifecycle::table_entries
