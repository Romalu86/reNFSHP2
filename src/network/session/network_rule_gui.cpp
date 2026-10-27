#include "nfs/network/session/network_rules.hpp"
#include "nfs/gui/events/gui_events.hpp"
namespace nfs::network::rules {
using namespace packets;

void choice_values(Dword *owner, Dword identifier, Dword count, void *values) {
    if (void *widget = at<void *>(owner, 40)) {
        Dword descriptor[] = {count, address_of(values)};
        gui::events::propagate(widget, 102, identifier, address_of(descriptor), 1);
    }
}

void choice_label(Dword *owner, Dword identifier, const char *label) {
    if (void *widget = at<void *>(owner, 40))
        gui::events::propagate(widget, 105, identifier, address_of(label), 1);
}

void choice_enabled(Dword *owner, Dword identifier, std::uint8_t enabled) {
    if (void *widget = at<void *>(owner, 40))
        gui::events::propagate(widget, 109, identifier, enabled, 1);
}

void choice_selected(Dword *owner, Dword identifier, Dword value) {
    if (void *widget = at<void *>(owner, 40))
        gui::events::propagate(widget, 103, identifier, value, 1);
}

} // namespace nfs::network::rules
