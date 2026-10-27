#include "../lifecycle/gui_configuration_values.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstdio>
namespace nfs::gui::engine::pending {
// This overload records INT rather than LONG in the document.
Dword write_int(void *configuration, const char *key, int value, int fallback) {
    char buffer[256];
    std::sprintf(buffer, "%d", value);
    String text;
    string_construct_ascii(&text, buffer);
    Dword result;
    try {
        result = game::runtime::invoke<Dword>(configuration, 7, key, &text, "INT", Dword(0),
                                             Dword(value == fallback));
    } catch (...) {
        string_destruct(&text);
        throw;
    }
    string_destruct(&text);
    return result;
}

Dword write_ulong(void *configuration, const char *key, Dword value, Dword fallback) {
    char buffer[256];
    std::sprintf(buffer, "%lu", static_cast<unsigned long>(value));
    String text;
    string_construct_ascii(&text, buffer);
    Dword result;
    try {
        result = game::runtime::invoke<Dword>(configuration, 7, key, &text, "ULONG", Dword(0),
                                             Dword(value == fallback));
    } catch (...) {
        string_destruct(&text);
        throw;
    }
    string_destruct(&text);
    return result;
}
} // namespace nfs::gui::engine::pending
