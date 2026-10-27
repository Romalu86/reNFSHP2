#include "../../network/session/network_pursuit_runtime.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {

const ProfileColour profile_colours[14] = {{0, 1, "black", 4284769380u},
                                           {1, 2, "blue", 4278190250u},
                                           {2, 4, "red", 4289331200u},
                                           {3, 8, "yellow", 4294967120u},
                                           {4, 16, "white", 4292927712u},
                                           {5, 32, "silver", 4292927712u},
                                           {6, 64, "gold", 4286611488u},
                                           {7, 128, "purple", 4289331370u},
                                           {8, 256, "green", 4278233600u},
                                           {9, 512, "brown", 4289351680u},
                                           {10, 1024, "orange", 4294934528u},
                                           {11, 2048, "light blue", 4283453695u},
                                           {12, 4096, "light green", 4283498320u},
                                           {Dword(-1), 8192, "unknown", 4284769380u}};
static_assert(sizeof(ProfileColour) == 16);

Dword police_colour_mask(int colour) {
    const ProfileColour *entry = profile_colours;
    do {
        if (entry->value == Dword(colour))
            return entry->mask;
        ++entry;
    } while (entry->value != Dword(-1));
    return 8192;
}

void select_police_voice(void *owner, Byte secondary, Dword actor) {
    const Dword primary = at<Dword>(owner, 64);
    Dword selected = 0;
    while (selected == primary || selected == at<Dword>(owner, 68)) {
        ++selected;
        if (int(selected) >= 3) {
            selected = runtime::emitter_random_word() % 3;
            break;
        }
    }
    if (selected == Dword(-1))
        selected = runtime::emitter_random_word() % 3;
    if (actor == at<Dword>(owner, 72))
        selected = primary;
    if (!secondary) {
        at<Dword>(owner, 64) = selected;
        at<Dword>(owner, 72) = actor;
        at<Byte>(owner, 52) = 0;
    } else
        at<Dword>(owner, 68) = selected;
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
