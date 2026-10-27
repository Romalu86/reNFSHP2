#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "network_lifecycle_runtime.hpp"
#include <cstdlib>
namespace nfs::network::session::pending {
namespace {
players::Visitor spectator_slot_comparer{};
ui::PlayerRecord spectator_player{};
Byte spectator_player_guard{};
} // namespace

static void finalize_spectator_comparer() {
    players::destroy_slot(&spectator_slot_comparer);
}

static void initialize_spectator_comparer() {
    players::construct_slot(&spectator_slot_comparer);
    std::atexit(finalize_spectator_comparer);
}
// Complete RET.
static void finalize_spectator_player() {
}

Byte spectator_team(int slot) {
    if (!(spectator_player_guard & 1)) {
        spectator_player_guard |= 1;
        ui::initialize_car_settings(&spectator_player.car);
        std::atexit(finalize_spectator_player);
    }
    if (queue::copy_matching(player_queue(), reinterpret_cast<Dword>(&slot),
                             reinterpret_cast<Dword>(&spectator_player),
                             reinterpret_cast<Dword *>(&spectator_slot_comparer)))
        return at<Byte>(&spectator_player, 120);
    return 255;
}
} // namespace nfs::network::session::pending
NFS_KERNEL_INITIALIZER(startup_initialize_spectator_comparer, ".CRT$XCU_NFS_00690264",
                       nfs::network::session::pending::initialize_spectator_comparer);
