#include "nfs/network/session/network_player_state.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::network::players {
// The first cleared array is deliberately unaligned.
void reset_client_updates(Dword *owner) {
    for (unsigned i = 0; i < 8; ++i)
        field<Dword>(owner, 2105 + i * 4) = 0;
    for (unsigned i = 0; i < 21; ++i)
        field<Dword>(owner, 436 + i * 4) = 0;
    for (unsigned i = 0; i < 393; ++i)
        field<Dword>(owner, 520 + i * 4) = 0;
    field<std::uint16_t>(owner, 378) = 65535;
    field<Dword>(owner, 60) = 0;
    field<std::uint8_t>(owner, 2104) = 0;
}
 
Dword __fastcall reset_slot(Visitor *, void *, ui::PlayerRecord *player, Dword, Dword, Dword) {
    auto *manager = lifecycle::manager;
    const auto slot = player->slot;
    if (slot >= 0) {
        ui::CarSettings empty{};
        if (static_cast<Dword>(slot) < 24)
            kernel::memory_copy(reinterpret_cast<std::uint8_t *>(manager->slots[72 / 4]) + 248 +
                                    slot * 84,
                                &empty, 84);
        kernel::memory_fill(&player->car, 0, 84);
        reinterpret_cast<std::uint8_t *>(player)[120] = 0;
        player->slot = -1;
    }
    if (player->phase == 9) {
        auto *owner = reinterpret_cast<Dword *>(manager->slots[112 / 4]);
        player->phase = 8;
        if (field<Dword>(owner, 68) &&
            field<std::uint16_t>(owner, 376) == static_cast<std::uint16_t>(player->identifier)) {
            field<std::uint16_t>(owner, 380) = 8;
            reset_client_updates(owner);
        }
    }
    return 1;
}

Dword __fastcall start_vehicle(Visitor *, void *, ui::PlayerRecord *player, Dword, Dword, Dword) {
    if (player->phase == 9 && player->slot >= 0 && field<Dword>(&player->car, 4)) {
        auto *vehicles = reinterpret_cast<Dword *>(lifecycle::manager->slots[68 / 4]);
        if (void *vehicle = field<void *>(vehicles, 20 + player->slot * 4))
            field<std::uint8_t>(vehicle, 776) = 1;
    }
    return 1;
}

Dword __fastcall prepare_slot(Visitor *, void *, ui::PlayerRecord *player, Dword, Dword, Dword) {
    auto *manager = lifecycle::manager;
    if (player->phase == 9 && player->slot >= 0) {
        auto *owner = reinterpret_cast<Dword *>(manager->slots[112 / 4]);
        field<Dword>(&player->car, 4) =
            (!field<Dword>(owner, 68) || field<std::int16_t>(owner, 378) != player->slot) ? 1
                                                                                             : 0;
        auto *rules = reinterpret_cast<Dword *>(manager->slots[72 / 4]);
        if (static_cast<Dword>(player->slot) < 24)
            kernel::memory_copy(reinterpret_cast<std::uint8_t *>(rules) + 248 +
                                    player->slot * 84,
                                &player->car, 84);
        field<Dword>(rules, 168) += 1;
    }
    return 1;
}

Dword __fastcall count_slot(Visitor *, void *, ui::PlayerRecord *player, Dword, Dword, Dword *total) {
    if (player->phase == 9)
        ++*total;
    return 1;
}

Dword __fastcall compare_slot(Visitor *, void *, const std::int16_t *slot,
                             const ui::PlayerRecord *player) {
    return *slot == player->slot;
}
} // namespace nfs::network::players
