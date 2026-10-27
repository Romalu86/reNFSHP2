#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include "nfs/resources/localization/region.hpp"
#include <shlobj.h>
#include <cstring>
#include <cstdio>

namespace nfs::services {

Dword create_user_directory(void *object) {
    char path[520]{};
    if (!SHGetSpecialFolderPathA(nullptr, path, 5, TRUE))
        return 0;
    std::strcat(path, "\\EA Games\\");
    if (!CreateDirectoryA(path, nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
        return 0;
    std::strcat(path, region::get() == 1 ? "Need For Speed poursuite infernale 2\\"
                                         : "Need For Speed Hot Pursuit 2\\");
    if (!CreateDirectoryA(path, nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
        return 0;
    std::strcpy(static_cast<char *>(object) + 3124, path);
    return 1;
}

void initialize_paths(void *object) {
    kernel::memory_zero(static_cast<Byte *>(object) + 4, 8320);
    char local[512] = ".\\", installed[512];
    std::strncpy(installed, files::cache::application::directory(), 511);
    installed[511] = 0;
    unsigned length = static_cast<unsigned>(std::strlen(local));
    if (local[length - 1] != '\\') {
        local[length] = '\\';
        local[length + 1] = 0;
    }
    length = static_cast<unsigned>(std::strlen(installed));
    if (installed[length - 1] != '\\') {
        installed[length] = '\\';
        installed[length + 1] = 0;
    }
    struct PathEntry {
        const char *suffix;
        unsigned index;
        Byte installed;
    };
    const PathEntry entries[] = {{"", 0, 0},
                                 {"EAGL\\", 1, 0},
                                 {"TRACKS\\", 2, 0},
                                 {"ACTORS\\", 3, 0},
                                 {"AUDIO\\MUSIC\\", 4, 0},
                                 {"AUDIO\\SFX\\", 5, 0},
                                 {"AUDIO\\SPEECH\\", 6, 0},
                                 {"MOVIES\\", 7, 0},
                                 {"CARS\\", 8, 0},
                                 {"FRONTEND\\", 9, 0},
                                 {"FRONTEND\\GUI\\", 10, 0},
                                 {"..\\..\\ART\\UI\\", 11, 1},
                                 {"SAVEDATA\\", 12, 1},
                                 {"TEXT\\", 13, 0},
                                 {"AI\\", 14, 0},
                                 {"Controllers\\", 15, 1}};
    for (const auto &entry : entries)
        std::sprintf(static_cast<char *>(object) + 4 + entry.index * 260, "%s%s",
                     entry.installed ? installed : local, entry.suffix);
    if (!create_user_directory(object) && !lifecycle::startup_error)
        lifecycle::startup_error = 8;
}
} // namespace nfs::services
