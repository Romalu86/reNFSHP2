#pragma once
#include "nfs/core/types/kernel.hpp"
namespace nfs::network::session::pending::updates {
using kernel::Dword;
using Callback = void(__cdecl *)(Dword, Dword, const char *, Dword, const char *, Dword);
Dword request_version(Dword, const char *, Dword, Callback, Dword, Dword);
Dword request_motd(Dword, Dword, const char *, Dword, Dword);
} // namespace nfs::network::session::pending::updates
