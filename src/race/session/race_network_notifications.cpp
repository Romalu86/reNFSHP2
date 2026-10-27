#include "race_session.hpp"
#include "nfs/network/session/network_gui.hpp"
#include <cstdlib>
namespace nfs::application::pending {
namespace {
network::ui::PlayerRecord player_lookup;
Byte player_lookup_initialized;
struct NoticeFlags {
    Byte reliable : 1;
    Byte local : 1;
    Byte reserved : 6;
};
struct TextNotice {
    Byte opcode, padding[3];
    Dword reference;
    int recipient;
    Dword sender;
    NoticeFlags flags;
    Byte tail[3];
};
static_assert(sizeof(TextNotice) == 20 && offsetof(TextNotice, flags) == 16);
} // namespace
// The saved atexit callback is a complete RET leaf.
void release_player_lookup() {
}

std::uint16_t lookup_player_identifier(void *game, Dword slot) {
    if (!(player_lookup_initialized & 1)) {
        player_lookup_initialized |= 1;
        network::ui::initialize_car_settings(&player_lookup.car);
        std::atexit(release_player_lookup);
    }
    namespace q = lifecycle::construction::network::queue;
    if (q::copy_matching(network::server_queue(static_cast<Dword *>(game)),
                         reinterpret_cast<Dword>(&slot), reinterpret_cast<Dword>(&player_lookup),
                         reinterpret_cast<Dword *>(&network::players::slot_comparer)))
        return std::uint16_t(player_lookup.identifier);
    return 65535;
}

void notify_eliminated_participant(void *server, Dword slot) {
    struct EliminationNotice {
        Byte opcode, padding[3];
        Dword slot;
    } notice;
    notice.opcode = 31;
    notice.slot = slot;
    network::broadcast(static_cast<Dword *>(server), &notice, sizeof(notice), 32, nullptr);
    const int recipient = std::int16_t(lookup_player_identifier(system(104), slot));
    if (recipient != -1) {
        TextNotice text;
        text.opcode = 48;
        text.padding[0] = 0; // High byte of the preceding descriptor's saved type word.
        text.reference = 2052;
        text.recipient = recipient;
        text.sender = Dword(-2);
        // The saved stack slot was the preceding descriptor's zero exclude word.
        at<Dword>(&text, 16) = 3;
        network::broadcast(static_cast<Dword *>(server), &text, sizeof(text), 0, nullptr);
    }
}

void notify_finished_participant(void *server, Dword slot) {
    const int recipient = std::int16_t(lookup_player_identifier(system(104), slot));
    if (recipient == -1)
        return;
    TextNotice text;
    text.opcode = 48;
    text.reference = 1641;
    text.recipient = recipient;
    text.sender = Dword(-2);
    // Set only these two bits; leave the remaining packet bytes untouched.
    text.flags.reliable = 1;
    text.flags.local = 1;
    network::broadcast(static_cast<Dword *>(server), &text, sizeof(text), 0, nullptr);
    if (!at<Dword>(server, 36)) {
        const Dword delay = at<Dword>(server, 132);
        if (delay) {
            at<Dword>(server, 40) = 0;
            at<Dword>(server, 36) = network::transport_time() + 250 + delay;
        }
    }
}
} // namespace nfs::application::pending
