#pragma once
#include "nfs/gui/lifecycle/gui.hpp"
#include <cstdarg>
namespace nfs::gui {
int estimate_ascii(const char *, const Dword *);
int estimate_wide(const wchar_t *, const Dword *);
int string_format(String *, const String *, ...);
int string_format_ascii(String *, const char *, ...);
void string_substring(const String *, int, int, String *);
bool string_equal(const String *, const String *);
String *concatenate(String *, const String *, const String *);
String *append_ascii_value(String *, const String *, const char *);
String *prepend_ascii_value(String *, const char *, const String *);
} // namespace nfs::gui
