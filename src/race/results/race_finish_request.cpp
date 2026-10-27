#include "../session/race_runtime.hpp"
namespace nfs::application::pending {

void clear_pursuit_finish_flags() {
    for (unsigned address = 7352584; address < 7352816; address += 116)
        game::runtime::ai::lifecycle::state<Dword>(address) = 0;
}

void __fastcall request_race_finish(void *race, void *, Dword delay) {
    if (network::session_active() && !at<Dword>(system(108), 44) &&
        at<std::int16_t>(system(104), 10) != 6)
        return;
    if (at<int>(race, 34596) >= 0)
        return;
    at<Dword>(race, 34596) = delay * 60 + race_clock();
    for (Dword slot = 0; slot < 24; ++slot) {
        bool finish = false;
        if (!network::session_active()) {
            const int kind = participant(race, slot)->kind;
            if (kind == 1 || kind == 2)
                finish = true;
        } else {
            void *entry = bytes(system(72)) + 248 + slot * 84;
            if (at<Dword>(entry, 0))
                finish = true;
        }
        if (!finish) {
            const int kind = participant(race, slot)->kind;
            finish = kind == 3 || kind == 2;
        }
        if (finish)
            finish_participant(race, slot);
    }
    clear_pursuit_finish_flags();
}
} // namespace nfs::application::pending
