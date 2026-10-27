#include "nfs/network/session/network_rules.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/core/console/console.hpp"
#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
#include <cstdio>
namespace nfs::network::rules::pending {
using namespace packets;
void *language_data{};
Dword unicode_language{};

Dword uses_unicode_language() {
    return unicode_language;
}
extern Dword &narrow_language_only; // Shared launch option.

void *load_language_file(const char *path, Dword *bytes, Dword flags) {
    return files::file::try_load_with_length(path, bytes, flags);
}

void clear_language() {
    if (language_data)
        gui::application_release(language_data);
    language_data = nullptr;
}

void load_language(Dword language) {
    if (language_data)
        return;
    if (static_cast<int>(language) >= 7) {
        kernel::print::check_console_write("Language #%d >= max(%d)\n", language, 7);
        return;
    }
    static const char *names[] = {"ENG", "FRE", "GER", "ITA", "SPA", "SWE", "KOR"};
    char path[260];
    Dword length = 0;
    void *loaded_text;
    if (!narrow_language_only) {
        const char *root = reinterpret_cast<const char *>(system(4)) + 3384;
        std::sprintf(path, "%sunicodeText.%s", root, names[language]);
        loaded_text = load_language_file(path, &length, lifecycle::level_pool | 256);
        language_data = loaded_text;
        if (loaded_text) {
            unicode_language = 1;
            goto validate;
        }
    }
    {
        const char *root = reinterpret_cast<const char *>(system(4)) + 3384;
        std::sprintf(path, "%stext.%s", root, names[language]);
    }
    loaded_text = load_language_file(path, &length, lifecycle::level_pool | 256);
    language_data = loaded_text;
    unicode_language = 0;
    if (!loaded_text) {
        kernel::print::check_console_write("%s is missing!", path);
        return;
    }
validate:
    const int source_bytes = at<int>(loaded_text, 10512);
    if (source_bytes < 0 || static_cast<int>(length) < source_bytes)
        kernel::print::check_console_write("%s file is outta sync with source code! (A)", path);
}
} // namespace nfs::network::rules::pending
