#include "frontend_runtime.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/network/session/network_rules.hpp"
#include <cstring>
namespace nfs::application::pending {
// Forward to the shared profile owner.
void *player_preferences(void *rules, Dword player) {
    return network::rules::profile(static_cast<Dword *>(rules), player);
}

gui::String *copy_preference_name(void *preferences, gui::String *output) {
    return gui::string_construct_ascii(output, static_cast<char *>(preferences));
}

void set_preference_name(void *preferences, const gui::String *name) {
    LocalFrontendString copy(name);
    if (gui::string_make_narrow(&copy.value) &&
        network::packets::string_character(&copy.value, 0)) {
        std::strncpy(static_cast<char *>(preferences), gui::string_ascii(&copy.value), 25);
        at<Byte>(preferences, 25) = 0;
    } else {
        at<Byte>(preferences, 0) = 0;
    }
}

void set_preference_network_name(void *preferences, const gui::String *name) {
    LocalFrontendString copy(name);
    if (gui::string_make_narrow(&copy.value) &&
        network::packets::string_character(&copy.value, 0)) {
        std::strncpy(static_cast<char *>(preferences) + 26, gui::string_ascii(&copy.value), 25);
        at<Byte>(preferences, 25) = 0;
    } else {
        at<Byte>(preferences, 26) = 0;
    }
}
} // namespace nfs::application::pending
