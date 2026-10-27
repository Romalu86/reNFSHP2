#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "network_lifecycle_runtime.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/network/session/network_construction.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "../packets/network_packet_x87.hpp"
#include <cstdlib>
#include <cmath>
namespace nfs::network::session {
players::Visitor timeout_visitor{}, initial_car_visitor{}, remote_car_visitor{},
    authentication_visitor{};

namespace {
extern const Dword timeout_methods[], initial_car_methods[], remote_car_methods[],
    authentication_methods[];
bits::Stream spectator_encoder{};
Byte spectator_encoder_guard{};
} // namespace
// Saved complete RET finalizer.
static void finalize_spectator_encoder() {
}

static Dword __fastcall expire_route(players::Visitor *, void *, void *route, Dword, Dword,
                                    Dword context) {
    const Dword now = transport_time();
    void *owner = reinterpret_cast<void *>(context);
    if (!at<Dword>(route, 4) && at<Dword>(route, 12)) {
        const int timeout = at<int>(owner, 104);
        if (timeout < static_cast<std::int32_t>(now - at<Dword>(route, 20))) {
            while (remove_server_client(server(), nullptr, at<std::uint16_t>(route, 0))) {
            }
            boot_route(route, 7);
            return 1;
        }
        const int half = timeout / 2;
        if (half < static_cast<std::int32_t>(now - at<Dword>(route, 20)) &&
            (half < static_cast<std::int32_t>(now - at<Dword>(route, 28)) ||
             at<Byte>(route, 25) != 39)) {
            alignas(4) Byte packet[8];
            packet[0] = 39;
            at<Dword>(packet, 4) = now;
            packets::send(route, packet, sizeof(packet));
        }
    }
    return 1;
}

static Dword __fastcall send_initial_car(players::Visitor *, void *, ui::PlayerRecord *player, Dword,
                                        Dword, Dword) {
    if (player->phase == 9 && player->slot >= 0) {
        alignas(4) Byte packet[1576];
        lifecycle::construction::objects::client_state(packet + 4);
        packet[0] = 12;
        if (void *car = packets::vehicle(player->slot)) {
            for (unsigned word = 0; word < 393; ++word)
                at<Dword>(packet, 4 + word * 4) = at<Dword>(car, 4 + word * 4);
            alignas(4) Byte route[304];
            if (queue::copy_matching(route_queue(), reinterpret_cast<Dword>(bytes(player) + 2),
                                     reinterpret_cast<Dword>(route), nullptr))
                packets::send(route, packet, sizeof(packet));
        }
    }
    return 1;
}

static Dword __fastcall send_spectator_car(players::Visitor *, void *, ui::PlayerRecord *player,
                                          Dword, Dword, Dword vehicle) {
    void *car = reinterpret_cast<void *>(vehicle);
    if (!car || !services::pending::car_is_remote(car))
        return 0;
    if (player->phase != 7)
        return 1;
    alignas(4) Byte state[80];
    for (unsigned word = 0; word < 20; ++word)
        at<Dword>(state, word * 4) = at<Dword>(car, 3400 + word * 4);
    if (at<Dword>(state, 0) == Dword(-1))
        return 1;
    for (unsigned word = 0; word < 20; ++word)
        at<Dword>(state, word * 4) = at<Dword>(car, 3400 + word * 4);
    alignas(4) Byte packet[40];
    packet[0] = 35;
    packet[1] = at<Byte>(car, 1772);
    pack_client_car(state, packet + 4);
    alignas(4) Byte route[304];
    if (queue::copy_matching(route_queue(), reinterpret_cast<Dword>(bytes(player) + 2),
                             reinterpret_cast<Dword>(route), nullptr)) {
        if (!(packets::pending::compression_enabled() & 255))
            packets::send(route, packet, sizeof(packet), 4);
        else {
            if (!(spectator_encoder_guard & 1)) {
                spectator_encoder_guard |= 1;
                bits::construct(&spectator_encoder);
                std::atexit(finalize_spectator_encoder);
            }
            bits::reset(&spectator_encoder);
            bits::encode_server_car(&spectator_encoder, packet);
            const float quotient = float(static_cast<int>(spectator_encoder.written)) / 8.f;
            packets::send(route, &spectator_encoder, Dword(std::int64_t(std::ceil(quotient))), 4);
        }
    }
    return 1;
}
namespace pending {

static Dword *construct_authentication_record(Dword *record) {
    record[0] = record[1] = 0;
    at<Dword>(record, 320) = GetTickCount();
    at<Byte>(record, 8) = 0;
    at<Byte>(record, 17) = 0;
    at<Byte>(record, 324) = 0;
    at<Byte>(record, 325) = 0;
    return record;
}

bool local_route(std::uint16_t identifier) {
    alignas(4) Byte route[304];
    return queue::copy_matching(route_queue(), reinterpret_cast<Dword>(&identifier),
                                reinterpret_cast<Dword>(route), nullptr) &&
           at<Dword>(route, 4) == 1;
}

static void mark_authentication_player(ui::PlayerRecord *player) {
    const int identifier = std::int16_t(player->identifier);
    if (local_route(at<std::uint16_t>(player, 2)))
        return;
    Dword *record = packets::pending::authentication_record(identifier);
    if (!record) {
        auto *storage = static_cast<Dword *>(
            lifecycle::allocate(360, "GameSpy Client Object", lifecycle::level_pool));
        try {
            record = storage ? construct_authentication_record(storage) : nullptr;
        } catch (...) {
            game::runtime::track::pending::release_named(
                storage, "GameSpy Client Object", lifecycle::level_pool);
            throw;
        }
        record[0] = Dword(identifier);
        authentication::append_record(record);
    }
    at<Byte>(record, 324) = 1;
}
} // namespace pending

static Dword __fastcall mark_authentication(players::Visitor *, void *, ui::PlayerRecord *player,
                                           Dword, Dword, Dword) {
    pending::mark_authentication_player(player);
    return 1;
}

static void destroy_timeout_visitor(players::Visitor *owner) {
    owner->methods = lifecycle::construction::network::pending::visitor_interface_methods;
}

static void destroy_remote_car_visitor(players::Visitor *owner) {
    owner->methods = lifecycle::construction::network::pending::visitor_interface_methods;
}

static void destroy_initial_car_visitor(players::Visitor *owner) {
    owner->methods = lifecycle::construction::network::pending::visitor_interface_methods;
}

static void destroy_authentication_visitor(players::Visitor *owner) {
    owner->methods = lifecycle::construction::network::pending::visitor_interface_methods;
}

static void *__fastcall delete_timeout_visitor(players::Visitor *owner, void *, Dword flags) {
    destroy_timeout_visitor(owner);
    if (flags & 1)
        gui::application_release(owner);
    return owner;
}

static void *__fastcall delete_remote_car_visitor(players::Visitor *owner, void *, Dword flags) {
    destroy_remote_car_visitor(owner);
    if (flags & 1)
        gui::application_release(owner);
    return owner;
}

static void *__fastcall delete_initial_car_visitor(players::Visitor *owner, void *, Dword flags) {
    destroy_initial_car_visitor(owner);
    if (flags & 1)
        gui::application_release(owner);
    return owner;
}

static void *__fastcall delete_authentication_visitor(players::Visitor *owner, void *, Dword flags) {
    destroy_authentication_visitor(owner);
    if (flags & 1)
        gui::application_release(owner);
    return owner;
}
namespace {
const Dword timeout_methods[]{reinterpret_cast<Dword>(expire_route),
                             reinterpret_cast<Dword>(delete_timeout_visitor)};
const Dword initial_car_methods[]{reinterpret_cast<Dword>(send_initial_car),
                                 reinterpret_cast<Dword>(delete_initial_car_visitor)};
const Dword remote_car_methods[]{reinterpret_cast<Dword>(send_spectator_car),
                                reinterpret_cast<Dword>(delete_remote_car_visitor)};
const Dword authentication_methods[]{reinterpret_cast<Dword>(mark_authentication),
                                    reinterpret_cast<Dword>(delete_authentication_visitor)};
} // namespace

static void construct_timeout_visitor(players::Visitor *owner) {
    owner->methods = lifecycle::construction::network::pending::visitor_interface_methods;
    owner->methods = timeout_methods;
}

static void construct_remote_car_visitor(players::Visitor *owner) {
    owner->methods = lifecycle::construction::network::pending::visitor_interface_methods;
    owner->methods = remote_car_methods;
}

static void construct_initial_car_visitor(players::Visitor *owner) {
    owner->methods = lifecycle::construction::network::pending::visitor_interface_methods;
    owner->methods = initial_car_methods;
}

static void construct_authentication_visitor(players::Visitor *owner) {
    owner->methods = lifecycle::construction::network::pending::visitor_interface_methods;
    owner->methods = authentication_methods;
}

static void finalize_timeout_visitor() {
    destroy_timeout_visitor(&timeout_visitor);
}

static void finalize_remote_car_visitor() {
    destroy_remote_car_visitor(&remote_car_visitor);
}

static void finalize_initial_car_visitor() {
    destroy_initial_car_visitor(&initial_car_visitor);
}

static void finalize_authentication_visitor() {
    destroy_authentication_visitor(&authentication_visitor);
}

static void initialize_timeout_visitor() {
    construct_timeout_visitor(&timeout_visitor);
    std::atexit(finalize_timeout_visitor);
}

static void initialize_remote_car_visitor() {
    construct_remote_car_visitor(&remote_car_visitor);
    std::atexit(finalize_remote_car_visitor);
}

static void initialize_initial_car_visitor() {
    construct_initial_car_visitor(&initial_car_visitor);
    std::atexit(finalize_initial_car_visitor);
}

static void initialize_authentication_visitor() {
    construct_authentication_visitor(&authentication_visitor);
    std::atexit(finalize_authentication_visitor);
}

void update_server_cars(void *owner) {
    const float elapsed = frame_clock::elapsed, previous = at<float>(owner, 24);
    if (!packet_native::snapshot_due(elapsed, previous))
        return;
    at<float>(owner, 24) = elapsed;
    for (int slot = 0; slot < 24; ++slot)
        if (void *car = packets::vehicle(slot))
            if (packets::player_left(packets::race(), slot) & 255)
                queue::visit(player_queue(), 0, reinterpret_cast<Dword>(car),
                             reinterpret_cast<Dword *>(&remote_car_visitor), 0, nullptr);
}
namespace pending {

void mark_authentication_players(Dword *players) {
    queue::visit(players, 0, 0, reinterpret_cast<Dword *>(&authentication_visitor), 0, nullptr);
}
} // namespace pending
} // namespace nfs::network::session
NFS_KERNEL_INITIALIZER(startup_initialize_timeout_visitor, ".CRT$XCU_NFS_00690284",
                       nfs::network::session::initialize_timeout_visitor);
NFS_KERNEL_INITIALIZER(startup_initialize_remote_car_visitor, ".CRT$XCU_NFS_00690280",
                       nfs::network::session::initialize_remote_car_visitor);
NFS_KERNEL_INITIALIZER(startup_initialize_initial_car_visitor, ".CRT$XCU_NFS_00690288",
                       nfs::network::session::initialize_initial_car_visitor);
NFS_KERNEL_INITIALIZER(startup_initialize_authentication_visitor, ".CRT$XCU_NFS_0069025C",
                       nfs::network::session::initialize_authentication_visitor);
