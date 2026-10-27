#include "nfs/core/lifecycle/system_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
namespace nfs::lifecycle::runtime {
// The last nine bytes of the 16-byte record are untouched.
void reset_replay_record(void *memory) {
    auto *bytes = static_cast<volatile std::uint8_t *>(memory);
    *reinterpret_cast<volatile Dword *>(memory) = 0;
    bytes[4] = 0;
    bytes[5] = 0;
    bytes[6] = 0;
}
} // namespace nfs::lifecycle::runtime
namespace nfs::lifecycle::table_entries {

void __fastcall initialize_replay(System *object, void *) {
    auto *buffer = static_cast<std::uint8_t *>(
        files::archives::allocate_entry(345600, "replay buffer", replay_pool));
    if (buffer)
        for (unsigned i = 0; i < 21600; ++i)
            runtime::reset_replay_record(buffer + i * 16);
    reinterpret_cast<Dword *>(object)[1] = reinterpret_cast<Dword>(buffer);
}
// The sole callee  is a confirmed release-console RET 0.
void __fastcall unload_replay_level(System *, void *) {
}
} // namespace nfs::lifecycle::table_entries
