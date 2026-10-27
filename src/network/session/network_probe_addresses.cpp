#include "nfs/core/services/service_lifecycle.hpp"
#include <cstring>
namespace nfs::services::dependency::pending {
// Three address slots, separated by 1F8h. The other transport fields between the starts are not
// treated as independent scratch allocations.

// contiguous saved storage footprint, not a claim about string capacities.
alignas(4) unsigned char probe_address_storage[1508]{};

void set_bandwidth_probe_address(int index, const char *address) {
    if (index == 0)
        std::strcpy(reinterpret_cast<char *>(probe_address_storage), address);
    else if (index == 1)
        std::strcpy(reinterpret_cast<char *>(probe_address_storage + 504), address);
    else if (index == 2)
        std::strcpy(reinterpret_cast<char *>(probe_address_storage + 1008), address);
}

const char *bandwidth_probe_address(int index) {
    if (index == 0)
        return reinterpret_cast<const char *>(probe_address_storage);
    if (index == 1)
        return reinterpret_cast<const char *>(probe_address_storage + 504);
    if (index == 2)
        return reinterpret_cast<const char *>(probe_address_storage + 1008);
    return nullptr;
}
} // namespace nfs::services::dependency::pending
