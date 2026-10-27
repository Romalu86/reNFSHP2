#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
#include <cstring>

namespace nfs::services {
 
void pending::shutdown_network_identity() {
}

void set_server_race_setting(void *object, Dword value) {
    field<Dword>(object, 680) = value;
}

int find_catalog_car(const void *catalogue, const char *name) {
    for (int index = 0; index < field<int>(catalogue, 0); ++index) {
        const auto *record = static_cast<const Byte *>(catalogue) + 4 + index * 1228;
        if (!_stricmp(name, reinterpret_cast<const char *>(record + 4)))
            return field<int>(record, 0);
    }
    return -1;
}

Dword special_race_mode(const void *rules) {
    const int mode = field<int>(rules, 148);
    return (static_cast<Dword>(mode) & 4294967040u) | ((mode > 1 && mode < 5) ? 1u : 0u);
}

void set_gui_archive(void *archive) {
    pending::gui_archive = archive;
}

void set_gui_repeat_timing(Dword interval, Dword delay) {
    pending::gui_repeat_interval = interval;
    pending::gui_repeat_delay = delay;
}
// Only AL is defined as a boolean result by the callers.
bool feedback_selected(Dword device) {
    void *rules = lifecycle::manager->slots[72 / 4];
    for (Dword kind : {1u, 2u, 3u})
        for (unsigned offset : {2544u, 2548u}) {
            const Dword binding = field<Dword>(rules, offset);
            if (Byte(binding) == kind && (binding >> 28) == device)
                return true;
        }
    return false;
}
} // namespace nfs::services
