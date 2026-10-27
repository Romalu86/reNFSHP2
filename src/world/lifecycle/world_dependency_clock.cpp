#include "nfs/world/lifecycle/world_dependency.hpp"
namespace nfs::game::world::dependency {
 
void initialize_clock_origin() {
    FILETIME timestamp;
    GetSystemTimeAsFileTime(&timestamp);
    clock_origin = (std::uint64_t(timestamp.dwHighDateTime) << 32) + timestamp.dwLowDateTime;
}
// Compute elapsed FILETIME with unsigned subtraction and division.
Dword legacy_clock() {
    FILETIME timestamp;
    GetSystemTimeAsFileTime(&timestamp);
    std::uint64_t value = (std::uint64_t(timestamp.dwHighDateTime) << 32) + timestamp.dwLowDateTime;
    return Dword((value - clock_origin) / 10000);
}
} // namespace nfs::game::world::dependency


// the C++ callbacks. Keep this void callback ahead of compiler XCU entries.
#pragma section(".CRT$XCT", read)
extern "C" __declspec(allocate(".CRT$XCT")) void(__cdecl *const nfs_clock_startup)() =
    nfs::game::world::dependency::initialize_clock_origin;
#pragma comment(linker, "/include:_nfs_clock_startup")
