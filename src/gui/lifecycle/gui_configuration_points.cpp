#include "gui_configuration_values.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstdio>
namespace nfs::gui::engine::pending {
using game::runtime::invoke;

Dword read_point(void *configuration, const char *key, Point *out, const Point *fallback) {
    const String *value = invoke<const String *>(configuration, 5, key);
    if (value->length)
        return Dword(std::sscanf(string_ascii(value), "[ %d , %d ]", &out->x, &out->y));
    const int x = fallback->x, y = fallback->y;
    out->y = y;
    out->x = x;
    return 0;
}

Dword write_point(void *configuration, const char *key, const Point *value, const Point *fallback) {
    const Dword same = value->x == fallback->x && value->y == fallback->y;
    char buffer[256]{};
    std::sprintf(buffer, "[%d, %d]", value->x, value->y);
    String text;
    string_construct_ascii(&text, buffer);
    Dword result;
    try {
        result = invoke<Dword>(configuration, 7, key, &text, "POS", Dword(0), same);
    } catch (...) {
        string_destruct(&text);
        throw;
    }
    string_destruct(&text);
    return result;
}
} // namespace nfs::gui::engine::pending
