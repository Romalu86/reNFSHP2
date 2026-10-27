#pragma once
#include "nfs/gui/engine/gui_engine.hpp"
namespace nfs::gui::engine::pending {
struct FlagDefinition {
    const char *name;
    Dword reserved, value;
};
Dword parse_flags(const FlagDefinition *, const String *);
String *format_flags(String *, const FlagDefinition *, Dword);
Dword read_float(void *, const char *, float *, float);
Dword read_string(void *, const char *, String *, const String *);
Dword read_flags(void *, const char *, Dword *, const FlagDefinition *, Dword);
Dword read_enum(void *, const char *, Dword *, const FlagDefinition *, Dword);
Dword parse_enum(const FlagDefinition *, const String *);
String *format_enum(String *, const FlagDefinition *, Dword);
Dword read_colour(void *, const char *, Dword *, const Dword *);
Dword read_integer(void *, const char *, int *, int);
Dword read_int(void *, const char *, int *, int);
Dword read_reference_string(void *, const char *, String *, const String *);
Dword assign_symbolic_string(String *, const char *);
Dword write_ascii(void *, const char *, const char *, const char *, Dword, const char *);
Dword write_text(void *, const char *, const char *, const char *);
Dword write_integer(void *, const char *, int, int);
Dword write_int(void *, const char *, int, int);
Dword write_ulong(void *, const char *, Dword, Dword);
Dword write_float(void *, const char *, float, int, float);
Dword write_reference_string(void *, const char *, const String *, const String *);
Dword write_colour(void *, const char *, const Dword *, const Dword *);
Dword write_rectangle(void *, const char *, const Rectangle *, const Rectangle *);
Dword write_flags(void *, const char *, Dword, const FlagDefinition *, Dword);
Dword write_enum(void *, const char *, Dword, const FlagDefinition *, Dword);
} // namespace nfs::gui::engine::pending
