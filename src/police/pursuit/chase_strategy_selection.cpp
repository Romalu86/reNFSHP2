#include "../../network/session/network_pursuit_runtime.hpp"
#include "nfs/core/configuration/launch_options.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {
namespace {
bool roadblock_available(void *owner, void *chase) {
    if (at<Dword>(owner, 148))
        return false;
    for (unsigned address = 7352496; address < 7352728; address += 116) {
        void *other = &state<Dword>(address);
        if (other != chase && chase_active(other) && at<int>(other, 16) != -1)
            return false;
    }
    return cars::idle_roadblock_cop_count(owner) > 1 &&
           cars::free_spike_available(bytes(owner) + 136);
}
void attach_first_idle_cop(void *owner, void *chase, Dword count) {
    for (Dword index = 0; index < count; ++index) {
        void *cop = at<void *>(owner, index * 4);
        if (at<Dword>(cop, 572))
            continue;
        void *target = at<void *>(chase, 44);
        at<Dword>(cop, 580) = 1;
        if (target) {
            void *session = system(92);
            const Dword participant = network::world::vehicle_slot(target);
            record_support_request(at<void *>(session, 4), participant, 3);
        }
        attach_chase_cop(chase, cop);
        break;
    }
}
} // namespace

void choose_chase_level_three(void *owner, void *chase, void *cop) {
    Byte helicopter = launch::field<Dword>(8627932) != 0;
    Dword strategy = 2;
    if (!helicopter) {
        strategy = state<Byte>(7352729) ? 1 : ai::random_word() & 3;
        if (strategy == 1 && !roadblock_available(owner, chase))
            strategy = 2;
    }
    at<Dword>(cop, 580) = 1;
    at<std::uint16_t>(chase, 8) = std::uint16_t(strategy);
    Dword roadblock = Dword(-1);
    std::uint16_t backups = 0;
    switch (strategy) {
    case 0:
        backups = 1;
        break;
    case 1:
        roadblock = ai::random_word() % 3;
        at<void *>(owner, 148) = chase;
        break;
    case 2:
        helicopter = 1;
        break;
    case 3:
        attach_first_idle_cop(owner, chase, at<std::uint16_t>(owner, 80));
        break;
    }
    at<std::uint16_t>(chase, 12) = backups;
    at<Dword>(chase, 16) = roadblock;
    at<Byte>(chase, 20) = helicopter;
}

void choose_chase_level_four(void *owner, void *chase, void *cop) {
    Dword strategy = 3;
    if (!launch::field<Dword>(8627932)) {
        strategy = state<Byte>(7352729) ? 2 : ai::random_word() & 3;
        if (strategy == 2 && !roadblock_available(owner, chase))
            strategy = 3;
    }
    at<Dword>(cop, 580) = 1;
    const Dword count = at<std::uint16_t>(owner, 80);
    at<std::uint16_t>(chase, 8) = std::uint16_t(strategy);
    Dword roadblock = Dword(-1);
    Byte helicopter = 0;
    attach_first_idle_cop(owner, chase, count);
    if (strategy == 2) {
        roadblock = ai::random_word() % 3;
        at<void *>(owner, 148) = chase;
    } else if (strategy == 3)
        helicopter = 1;
    at<std::uint16_t>(chase, 12) = 0;
    at<Dword>(chase, 16) = roadblock;
    at<Byte>(chase, 20) = helicopter;
}

void choose_chase_level_five(void *owner, void *chase, void *cop) {
    Dword strategy = 0;
    if (!launch::field<Dword>(8627932)) {
        strategy = state<Byte>(7352729) ? 1 : ai::random_word() & 1;
        if (strategy == 1 && !roadblock_available(owner, chase))
            strategy = 0;
    }
    at<Dword>(cop, 580) = 1;
    const Dword count = at<std::uint16_t>(owner, 80);
    at<std::uint16_t>(chase, 8) = std::uint16_t(strategy);
    Byte helicopter = 0;
    Dword roadblock = Dword(-1);
    attach_first_idle_cop(owner, chase, count);
    if (!strategy)
        helicopter = 1;
    else if (strategy == 1) {
        roadblock = ai::random_word() % 3;
        at<void *>(owner, 148) = chase;
    }
    at<std::uint16_t>(chase, 12) = 0;
    at<Dword>(chase, 16) = roadblock;
    at<Byte>(chase, 20) = helicopter;
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
