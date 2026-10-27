#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include "nfs/network/session/network_gui.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "../rules/race_rules_x87.hpp"
#include "../../math/precision/retained_ai.hpp"
namespace nfs::network {
Dword race_pending(Dword *);
}
namespace nfs::game::runtime::ai::lifecycle::pending {
// Only AL is consumed by the caller.
Byte participant_enabled(void *race, Dword slot) {
    if (network::session_active()) {
        void *participant = slot < 24 ? bytes(system(72)) + 248 + slot * 84 : nullptr;
        return at<Dword>(participant, 0) != 0;
    }
    const int status = at<int>(race, slot * 988 + 1528);
    return status == 1 || status == 2;
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
namespace nfs::application::pending {
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
using kernel::Dword;

int count_racers_ahead(void *race, int slot) {
    void *participant = network::packets::pending::find_race_vehicle(slot);
    if (!participant)
        return 66;
    const float progress = float(game::runtime::ai::race_progress_x87::priority(participant));
    network::world::Iterator iterator;
    network::world::construct_iterator(&iterator, network::world::vehicle_registry);
    if (network::race_pending(
            reinterpret_cast<network::Dword *>(bytes(race) + 584 + Dword(slot) * 988)) &
        255)
        return static_cast<int>(at<Dword>(system(72), 168) -
                                at<Dword>(race, Dword(slot) * 988 + 1464) + 1);
    Dword ahead = 0;
    for (network::Dword value = network::world::first(&iterator); value;
         value = network::world::next(&iterator)) {
        void *entity = reinterpret_cast<void *>(value);
        if (entity == participant)
            continue;
        void *vehicle = network::world::vehicle_interface(value);
        if (!vehicle)
            continue;
        const network::Dword other_slot = network::world::vehicle_slot(vehicle);
        void *record = bytes(race) + 584 + other_slot * 988;
        if (!(network::race_pending(reinterpret_cast<network::Dword *>(record)) & 255) &&
            !at<game::runtime::Byte>(record, 940) && at<int>(record, 884) < 0 &&
            !at<game::runtime::Byte>(record, 941) && !at<game::runtime::Byte>(record, 942) &&
            static_cast<long double>(progress) <
                game::runtime::ai::race_progress_x87::priority(vehicle))
            ++ahead;
    }
    return static_cast<int>(ahead);
}
} // namespace nfs::application::pending
namespace nfs::game::world::pending {

Dword count_racers_ahead_for_slot(void *manager, int slot) {
    return Dword(application::pending::count_racers_ahead(at<void *>(manager, 4), slot));
}
} // namespace nfs::game::world::pending
