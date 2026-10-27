#include "../../frontend/flow/frontend_runtime.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include <cstring>
#include <cwchar>
namespace nfs::gui {

int string_find_wide(const String *text, const wchar_t *needle) {
    if (needle && text->wide)
        if (const auto *source = static_cast<const wchar_t *>(string_data(text)))
            if (const wchar_t *found = std::wcsstr(source, needle))
                return int(found - source);
    return -1;
}
// Uppercase both receivers before searching; their contents must not be cleared.
 
int string_find_ascii_folded(const String *text, const char *needle) {
    if (!needle)
        return -1;
    application::pending::LocalFrontendString haystack(text), pattern(needle);
    engine::uppercase(&haystack.value);
    engine::uppercase(&pattern.value);
    if (!string_data(&pattern.value))
        return -1;
    if (!pattern.value.wide)
        if (const char *narrow = string_ascii(&pattern.value)) {
            if (!haystack.value.wide)
                if (const char *source = string_ascii(&haystack.value))
                    if (const char *found = std::strstr(source, narrow))
                        return int(found - source);
            return -1;
        }
    if (!string_make_wide(&pattern.value))
        return -1;
    return string_find_wide(&haystack.value,
                            static_cast<const wchar_t *>(string_data(&pattern.value)));
}
} // namespace nfs::gui
