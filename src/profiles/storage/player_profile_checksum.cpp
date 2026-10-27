#include "player_profile.hpp"
namespace nfs::profiles {
namespace {
void *checksum_owner() {
    const auto &manager_slot = static_cast<lifecycle::Manager *const volatile &>(lifecycle::manager);
    return at<void *volatile>(manager_slot, 0);
}
} // namespace

void reset_checksum(void *owner) {
    at<volatile Dword>(owner, 1052) = at<volatile Dword>(owner, 1040);
}

void update_checksum(void *owner, const Byte *data, Dword count) {
    if (!count)
        return;
    do {
        const Dword reflected = at<volatile Dword>(owner, 1044);
        const Byte value = *static_cast<const volatile Byte *>(data);
        const Dword current = at<volatile Dword>(owner, 1052);
        if (!reflected) {
            const unsigned shift = (at<volatile Dword>(owner, 1028) - 8) & 31;
            const Dword index = ((current >> shift) & 255) ^ value;
            at<volatile Dword>(owner, 1052) =
                at<volatile Dword>(owner, 4 + index * 4) ^ (current << 8);
        } else {
            const Dword index = (current & 255) ^ value;
            at<volatile Dword>(owner, 1052) =
                at<volatile Dword>(owner, 4 + index * 4) ^ (current >> 8);
        }
        ++data;
    } while (--count);
}
Dword profile_checksum(SavedProfile *profile) {
    void *checksum = checksum_owner();
    reset_checksum(checksum);
    checksum = checksum_owner();
    update_checksum(checksum, profile->data, file_size);
    checksum = checksum_owner();
    const Dword final_xor = at<volatile Dword>(checksum, 1048);
    const Dword current = at<volatile Dword>(checksum, 1052);
    const Dword mask = at<volatile Dword>(checksum, 1032);
    return (current ^ final_xor) & mask;
}
} // namespace nfs::profiles
