#include "race_runtime.hpp"
namespace nfs::application::pending {
Dword choose_pursuit_grid_entry(void *, Dword, Dword);
Dword choose_racer_grid_entry(void *, Dword, Dword);
void __fastcall configure_race_grid(void *, void *, Dword);

void __fastcall select_participant_grid(void *race, void *, Dword slot) {
    void *entry = slot < 24 ? bytes(system(72)) + 248 + slot * 84 : nullptr;
    if (invoke<Byte>(race, 9))
        at<Dword>(entry, 52) = choose_pursuit_grid_entry(system(76), 0, 9);
    else
        at<Dword>(entry, 52) = choose_racer_grid_entry(system(76), 0, 9);
}

void __fastcall assign_grid_car(void *, void *, void *choices, Dword slot) {
    void *entry = slot < 24 ? bytes(system(72)) + 248 + slot * 84 : nullptr;
    Dword selected = Dword(-1);
    for (unsigned i = 0; i < 8 && selected == Dword(-1); ++i) {
        Dword &remaining = at<Dword>(choices, 96 + i * 4);
        if (remaining) {
            selected = at<Dword>(choices, i * 4);
            --remaining;
        }
    }
    at<Dword>(entry, 60) = selected;
}

void __fastcall assign_pursuit_grid_car(void *, void *, void *choices, Dword slot, Byte alternate) {
    Dword selected = Dword(-1);
    for (unsigned i = 0; i < 8 && selected == Dword(-1); ++i) {
        Dword &remaining = at<Dword>(choices, (alternate ? 160 : 128) + i * 4);
        if (remaining) {
            --remaining;
            selected = at<Dword>(choices, i * 4);
        }
    }
    void *entry = slot < 24 ? bytes(system(72)) + 248 + slot * 84 : nullptr;
    at<Dword>(entry, 60) = selected;
}

void __fastcall configure_paired_grid(void *race, void *, Dword settings) {
    configure_race_grid(race, nullptr, settings);
    const int limit = at<int>(system(72), 164);
    if (limit < 0 || limit > 5184000)
        *at<Dword *>(system(72), 102668 + 3 * 4) = 10800;
}
} // namespace nfs::application::pending

namespace nfs::application::pending {

void mark_event_arrest(void *progress, Dword slot) {
    const Dword row = (at<Dword>(progress, 0) >> 4) & 15;
    at<Byte>(progress, 308 + row * 24 + slot) = Byte(slot + 72);
}
} // namespace nfs::application::pending
