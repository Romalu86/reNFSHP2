#include "nfs/network/vehicles/network_snapshots.hpp"
#include <cstring>
namespace nfs::network::packets::pending {

int validate_car_sender(void *packet, std::uint16_t route, Dword *excluded_frame) {
    ui::PlayerRecord player;
    ui::initialize_car_settings(&player.car);
    if (copy_player(bytes(packet) + 2, &player) && at<std::uint16_t>(&player, 2) == route) {
        *excluded_frame = at<Byte>(packet, 39) & 63;
        return player.slot;
    }
    return -1;
}

void receive_client_car(Dword *owner, void *route, void *packet) {
    alignas(4) Byte state[40], position[20];
    Dword excluded_frame;
    state[1] = Byte(validate_car_sender(packet, at<std::uint16_t>(route, 0), &excluded_frame));
    state[0] = 35;
    if (at<Dword>(owner, 136))
        at<Byte>(packet, 38) |= 4;
    std::memcpy(state + 4, bytes(packet) + 4, 36);
    position[0] = 36;
    snapshots::unpack_position(state + 8, reinterpret_cast<float *>(position + 8));
    at<Dword>(position, 4) = at<Dword>(state, 4);
    position[1] = state[1];
    if (at<Dword>(owner, 48))
        receive_position(network_game(), position);
    else if (!at<Dword>(route, 4))
        receive_car_state(network_game(), state);
    struct Descriptor {
        void *state;
        Dword state_bytes;
        void *position;
        Dword position_bytes;
        void *exclude;
        Dword excluded_frame;
    } descriptor{state, 40, position, 20, route, excluded_frame};
    Dword copy[76];
    for (int index = 0; index < static_cast<int>(queue::count(route_queue(owner))); ++index)
        if (queue::copy_nth(route_queue(owner), 0, index, 0, address_of(copy), nullptr, 0, nullptr))
            relay_car_route(copy, &descriptor);
}
} // namespace nfs::network::packets::pending
