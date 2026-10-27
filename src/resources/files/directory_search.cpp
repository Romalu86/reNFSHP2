#include "nfs/resources/files/directory_search.hpp"
#include <cstring>
namespace nfs::files {

std::uint8_t find_first(const char *pattern, DirectorySearch *search) {
    WIN32_FIND_DATAA entry;
    HANDLE handle = FindFirstFileA(pattern, &entry);
    if (handle == INVALID_HANDLE_VALUE) {
        search->handle = INVALID_HANDLE_VALUE;
        return 0;
    }
    search->attributes = entry.dwFileAttributes;
    search->size = entry.nFileSizeLow;
    search->handle = handle;
    std::strcpy(search->name, entry.cFileName);
    return 1;
}

std::uint8_t find_next(DirectorySearch *search) {
    WIN32_FIND_DATAA entry;
    if (!FindNextFileA(search->handle, &entry))
        return 0;
    search->attributes = entry.dwFileAttributes;
    search->size = entry.nFileSizeLow;
    std::strcpy(search->name, entry.cFileName);
    return 1;
}

void find_close(DirectorySearch *search) {
    if (search->handle != INVALID_HANDLE_VALUE)
        FindClose(search->handle);
}
} // namespace nfs::files
