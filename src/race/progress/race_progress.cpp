#include "nfs/network/session/network_runtime.hpp"
namespace nfs::network {
// Reset both 100-entry position lists; no race rules inferred.
void reset_race_progress(Dword *race) {
    field<Dword>(race, 16) = 0;
    field<Dword>(race, 12) = 4294967295u;
    field<Dword>(race, 936) = 4294967295u;
    for (unsigned i = 0; i < 100; ++i) {
        field<Dword>(race, 56 + i * 4) = 4294967295u;
        field<Dword>(race, 456 + i * 4) = 4294967295u;
    }
}
} // namespace nfs::network
