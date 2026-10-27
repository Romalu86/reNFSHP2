#include "nfs/core/configuration/launch_options.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/resources/ini/ini.hpp"
#include <cctype>
namespace nfs::launch::settings {

Dword open(const char *path) {
    cursor = static_cast<char *>(files::file::try_load(path, ini::load_pool));
    allocation = cursor;
    if (!cursor) {
        remaining = 0;
        return 0;
    }
    remaining = static_cast<int>(kernel::memory_size(cursor));
    return 1;
}

void close() {
    if (allocation) {
        kernel::memory_free(allocation);
        allocation = cursor = nullptr;
    }
}

} // namespace nfs::launch::settings
