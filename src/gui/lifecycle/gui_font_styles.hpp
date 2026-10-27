#pragma once
#include "nfs/gui/engine/gui_engine.hpp"
namespace nfs::gui::engine::pending {
struct FontStyleRecord {
    const Destroy *methods;
    String name;
    Dword handle;
    FontStyle style;
};
static_assert(sizeof(FontStyleRecord) == 56 && offsetof(FontStyleRecord, style) == 28);
struct FlagDefinition;
extern const FlagDefinition font_flags[];
FontStyleRecord *create_font_style_record(const String *);
void construct_configuration_for_file(void *, const char *);
int compare_font_style(const void *, const void *);
String *font_name(String *, int);
} // namespace nfs::gui::engine::pending
