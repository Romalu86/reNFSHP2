#include "nfs/network/session/network_rules.hpp"
#include "nfs/gui/events/gui_events.hpp"
namespace nfs::network::rules {
using namespace packets;

void *profile(Dword *owner, Dword slot) {
    return slot > 1 ? nullptr : bytes(owner) + 2488 + slot * 50088;
}

void *car_settings(Dword *owner, Dword slot) {
    return slot > 23 ? nullptr : bytes(owner) + 248 + slot * 84;
}
// Compare the fields as signed values.
int catalogue_index(int index) {
    if (index > 85)
        return 0;
    if (index > 61)
        return index - 62;
    if (index > 37)
        index -= 38;
    return index;
}
// Byte fields retain their three padding bytes.
void copy_vehicle(void *destination, void *source) {
    for (unsigned index = 0; index < 307; ++index) {
        bool single = index == 6 || index == 8;
        if (index >= 16 && index < 256)
            single = ((index - 16) % 12) == 4;
        if (index >= 256)
            single = index == 256 || index == 261 || index == 269 || index == 274 ||
                     index == 287;
        if (single)
            at<Byte>(destination, index * 4) = at<Byte>(source, index * 4);
        else
            at<Dword>(destination, index * 4) = at<Dword>(source, index * 4);
    }
}
} // namespace nfs::network::rules
