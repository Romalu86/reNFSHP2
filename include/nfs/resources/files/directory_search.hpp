#pragma once
#include "nfs/core/types/kernel.hpp"
namespace nfs::files {
using kernel::Dword;
struct DirectorySearch {
    Dword attributes, size;
    HANDLE handle;
    char name[260];
};
static_assert(sizeof(DirectorySearch) == 272 && offsetof(DirectorySearch, name) == 12);
std::uint8_t find_first(const char *, DirectorySearch *);
std::uint8_t find_next(DirectorySearch *);
void find_close(DirectorySearch *);
} // namespace nfs::files
