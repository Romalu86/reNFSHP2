#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include <cstring>
namespace nfs::services::pending {
// Preference name, remembered network name and command-line
// override have the precedence encoded in this entry, including the two reserved names.
void configure_client(void *owner, Dword player) {
    using namespace network::packets;
    const auto *empty = &lifecycle::construction::unresolved::shared_string;
    ScopedString chosen;
    gui::string_copy(&chosen.value, empty);
    void *preferences = player < 2 ? bytes(system(72)) + 2488 + player * 50088 : nullptr;
    ScopedString remembered;
    gui::string_copy(&remembered.value, empty);
    if (preferences) {
        ScopedString temporary;
        gui::string_assign(&temporary.value, static_cast<const char *>(preferences));
        gui::string_copy(&remembered.value, &temporary.value);
    }
    ScopedString network_name;
    gui::string_assign(&network_name.value, get_local_name());
    if (launch::field<Dword>(8627748)) {
        gui::string_assign(&chosen.value, reinterpret_cast<const char *>(&launch::options) + 908);
    } else if (!gui::string_equal(&network_name.value, empty)) {
        gui::string_copy(&chosen.value, &network_name.value);
    } else if (!gui::string_equal(&remembered.value, empty)) {
        bool reserved;
        {
            ScopedString placeholder;
            gui::string_reference(&placeholder.value, 2064);
            reserved = gui::string_equal(&remembered.value, &placeholder.value);
        }
        if (!reserved) {
            ScopedString placeholder;
            gui::string_reference(&placeholder.value, 454);
            reserved = gui::string_equal(&remembered.value, &placeholder.value);
        }
        if (!reserved)
            gui::string_copy(&chosen.value, &remembered.value);
    }
    if (gui::string_equal(&chosen.value, empty))
        gui::string_reference(&chosen.value, 2616);
    gui::string_make_narrow(&chosen.value);
    {
        ScopedString narrow;
        gui::string_copy(&narrow.value, &chosen.value);
        if (gui::string_make_narrow(&narrow.value) && string_character(&narrow.value, 0)) {
            std::strncpy(static_cast<char *>(preferences) + 26, gui::string_ascii(&narrow.value),
                         25);
            at<Byte>(preferences, 25) = 0;
        } else {
            at<Byte>(preferences, 26) = 0;
        }
    }
    if (!player)
        gui::string_copy(reinterpret_cast<gui::String *>(bytes(owner) + 4), &chosen.value);
}
} // namespace nfs::services::pending
