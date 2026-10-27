#include "nfs/network/session/network_gui.hpp"
#include "nfs/network/session/network_player_state.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstring>
namespace nfs::network {
namespace pending {} // namespace pending
namespace {
struct StatePacket {
    std::uint8_t opcode, padding;
    std::uint16_t identifier;
    std::int16_t phase, slot;
    ui::CarSettings car;
};
static_assert(sizeof(StatePacket) == 92);
void apply(const StatePacket *packet) {
    players::apply_state(server_queue(network_game()), packet);
}
void copy_car(void *destination, const void *source) {
    auto *out = static_cast<Dword *>(destination);
    auto *in = static_cast<const Dword *>(source);
    for (unsigned index = 0; index < 21; ++index)
        out[index] = in[index];
}
} // namespace
// The queue visitor is the server's second virtual interface.
void broadcast(Dword *owner, const void *data, Dword bytes, Dword flags, const void *exclude) {
    struct Descriptor {
        std::uint16_t type, padding;
        const void *data;
        Dword bytes, flags;
        const void *exclude;
    } message;
    message.type = 1;
    message.data = data;
    message.bytes = bytes;
    message.flags = flags;
    message.exclude = exclude;
    auto *visitor = owner ? reinterpret_cast<Dword *>(reinterpret_cast<std::uint8_t *>(owner) +
                                                     reinterpret_cast<Dword *>(owner[1])[2] + 4)
                          : nullptr;
    lifecycle::construction::network::queue::visit(
        server_queue(owner), 0, reinterpret_cast<Dword>(&message), visitor, 0, nullptr);
}

Dword race_pending(Dword *race) {
    const Dword value = field<Dword>(race, 880);
    return (value & 4294967040u) | Dword(static_cast<int>(value) > 0);
}

void mark_player_left(Dword *race, int slot, std::uint8_t value) {
    auto *participant = reinterpret_cast<Dword *>(reinterpret_cast<std::uint8_t *>(race) +
                                               Dword(slot) * 988 + 584);
    field<std::uint8_t>(participant, 942) = value;
    using Query = Dword(__thiscall *)(Dword *);
    const auto *methods = reinterpret_cast<const Query *>(race[0]);
    if (methods[4](race) & 255) {
        if (server()[44 / 4]) {
            using Update = void(__thiscall *)(Dword *, Dword);
            reinterpret_cast<const Update *>(race[0])[3](race, 4);
        }
    } else if (race[8655] == 1 && !(race_pending(participant) & 255)) {
        const int remaining = static_cast<int>(reinterpret_cast<const Query *>(race[0])[28](race));
        const int local = static_cast<int>(reinterpret_cast<const Query *>(race[0])[23](race));
        if (local > remaining)
            reset_race_progress(participant);
    }
}
// No added timeout or yield: the loop polls the signal.
void release_player_slot(Dword *owner, std::uint16_t identifier) {
    namespace q = lifecycle::construction::network::queue;
    ui::PlayerRecord player, last;
    ui::initialize_car_settings(&player.car);
    ui::initialize_car_settings(&last.car);
    auto *signal =
        reinterpret_cast<kernel::Signal *>(reinterpret_cast<std::uint8_t *>(owner) + 16);
    while (!kernel::signal_ready(signal))
        if (field<std::int16_t>(owner, 12) != 3)
            return;
    if (field<std::int16_t>(owner, 12) == 3 &&
        q::copy_matching(server_queue(network_game()), reinterpret_cast<Dword>(&identifier),
                         reinterpret_cast<Dword>(&player), nullptr) &&
        player.phase == 9 && player.slot >= 0) {
        ui::CarSettings empty{};
        const Dword slot = static_cast<Dword>(player.slot);
        if (slot < 24)
            copy_car(reinterpret_cast<std::uint8_t *>(lifecycle::manager->slots[72 / 4]) + 248 +
                         slot * 84,
                     &empty);
        StatePacket packet;
        ui::initialize_car_settings(&packet.car);
        packet.slot = -1;
        packet.identifier = identifier;
        copy_car(&packet.car, &empty);
        packet.opcode = 7;
        packet.phase = 8;
        apply(&packet);
        broadcast(owner, &packet, sizeof(packet), 32, nullptr);
        auto *rules = reinterpret_cast<Dword *>(lifecycle::manager->slots[72 / 4]);
        int last_slot = field<int>(rules, 168);
        if (last_slot > 0)
            field<int>(rules, 168) = last_slot - 1;
        last_slot = static_cast<int>(Dword(last_slot) - 1);
        if (last_slot > 0 && static_cast<int>(slot) < last_slot &&
            q::copy_matching(server_queue(network_game()), reinterpret_cast<Dword>(&last_slot),
                             reinterpret_cast<Dword>(&last),
                             reinterpret_cast<Dword *>(&players::slot_comparer))) {
            packet.slot = player.slot;
            packet.identifier = static_cast<std::uint16_t>(last.identifier);
            copy_car(&packet.car, &last.car);
            field<Dword>(&packet.car, 20) = 7 - slot;
            packet.opcode = 7;
            packet.phase = 9;
            apply(&packet);
            broadcast(owner, &packet, sizeof(packet), 32, nullptr);
        }
    }
    kernel::signal_set(signal);
}

Dword __fastcall remove_server_client(Dword *owner, void *, Dword identifier) {
    namespace q = lifecycle::construction::network::queue;
    ui::PlayerRecord player;
    ui::initialize_car_settings(&player.car);
    Dword *game = network_game();
    auto *comparer = game ? reinterpret_cast<Dword *>(reinterpret_cast<std::uint8_t *>(game) +
                                                     reinterpret_cast<Dword *>(game[1])[1] + 72)
                          : nullptr;
    if (!q::copy_matching(server_queue(game), reinterpret_cast<Dword>(&identifier),
                          reinterpret_cast<Dword>(&player), comparer))
        return 0;
    release_player_slot(owner, static_cast<std::uint16_t>(player.identifier));
    if (field<std::int16_t>(network_game(), 8) > 2 && player.slot >= 0) {
        field<Dword>(&player.car, 0) = 0;
        if (static_cast<Dword>(player.slot) < 24)
            copy_car(reinterpret_cast<std::uint8_t *>(lifecycle::manager->slots[72 / 4]) + 248 +
                         Dword(player.slot) * 84,
                     &player.car);
        auto *race = reinterpret_cast<Dword *>(lifecycle::manager->slots[92 / 4]);
        mark_player_left(reinterpret_cast<Dword *>(race[1]), player.slot, 1);
        race = reinterpret_cast<Dword *>(lifecycle::manager->slots[92 / 4]);
        mark_player_left(reinterpret_cast<Dword *>(race[2]), player.slot, 1);
    }
    struct Packet {
        std::uint8_t opcode, padding;
        std::uint16_t identifier;
    } packet;
    packet.opcode = 5;
    packet.identifier = static_cast<std::uint16_t>(player.identifier);
    gui::String line, name;
    gui::string_construct(&line);
    gui::string_construct(&name);
    try {
        gui::string_assign(&name, player.name);
        gui::string_make_wide(&name);
        gui::String pattern;
        gui::string_construct_reference(&pattern, 1611);
        try {
            gui::string_format(&line, &pattern, gui::string_data(&name));
        } catch (...) {
            gui::string_destruct(&pattern);
            throw;
        }
        gui::string_destruct(&pattern);
        gui::String notice;
        gui::string_construct_copy(&notice, &line);
        try {
            Dword *current_game = network_game();
            if (auto *frontend = field<Dword *>(current_game, 92))
                ui::notice(frontend, nullptr, &notice, -1, 0, 0, 0);
        } catch (...) {
            gui::string_destruct(&notice);
            throw;
        }
        gui::string_destruct(&notice);
        q::erase(server_queue(network_game()), reinterpret_cast<Dword>(&player), nullptr);
        broadcast(owner, &packet, sizeof(packet), 0, nullptr);
    } catch (...) {
        gui::string_destruct(&name);
        gui::string_destruct(&line);
        throw;
    }
    gui::string_destruct(&name);
    gui::string_destruct(&line);
    return 1;
}
} // namespace nfs::network
