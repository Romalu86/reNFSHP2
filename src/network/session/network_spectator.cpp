#include "network_lifecycle_runtime.hpp"
#include "../../graphics/cameras/camera_controller_runtime.hpp"
#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "../../race/session/race_runtime.hpp"
#include <cmath>
namespace nfs::application::pending {
void *race_participant_owner_view(void *, Dword);
}
namespace nfs::game::world::camera {

void bind_network_spectator(void *director, void *car) {
    if (!car)
        return;
    at<void *>(director, 300) = car;
    bind_mode_one(director, car);
    bind_mode_two(director, car);
    bind_mode_zero(director, car);
    void *mode = find_camera_mode(director, 3), *body = at<void *>(car, 1768);
    if (void *look = at<void *>(mode, 84))
        bind_mode_three_body(look, body);
    if (void *position = at<void *>(mode, 80)) {
        bind_mode_three_position(position, body);
        using kernel::numeric::Extended;
        const auto format = kernel::numeric::current_format();
        const Extended size(at<float>(body, 128), format);
        const auto distance = size - 1.f;
        const auto scaled = size * .85f;
        const float saved_scaled = float(scaled);
        set_mode_three_offset(position, 0, 1056964608,
                              scaled < distance ? saved_scaled : float(distance));
    }
    camera_layout::refresh_mode_three(director);
    for (Dword id : {8u, 9u, 4u}) {
        mode = find_camera_mode(director, id);
        if (void *look = at<void *>(mode, 84))
            at<void *>(look, 4) = offset(body, 680);
        if (id != 4)
            if (void *position = at<void *>(mode, 80))
                at<void *>(position, id == 8 ? 24 : 32) = body;
    }
}
} // namespace nfs::game::world::camera
namespace nfs::network::session {
namespace pending {
Byte spectator_team(int);
} // namespace pending

Dword select_spectator_vehicle(void *owner) {
    void *director = game::tick_services[0];
    if (at<std::int16_t>(network_game(), 8) != 5 || !director)
        return 0;
    auto active = [&](int slot) {
        void *participant = bytes(packets::race()) + 584 + Dword(slot) * 988;
        return !(race_pending(static_cast<Dword *>(participant)) & 255) &&
               !at<Byte>(participant, 942) && !at<Byte>(participant, 940) &&
               !at<Byte>(participant, 941) && at<int>(participant, 884) < 0;
    };
    if (void *car = at<void *>(director, 300))
        if (active(at<int>(car, 1772)))
            return 0;
    for (int slot = 0; slot < 24; ++slot)
        if (void *car = packets::vehicle(slot))
            if (pending::spectator_team(slot) == at<Byte>(owner, 2104) && active(slot)) {
                game::world::camera::bind_network_spectator(director, car);
                at<Dword>(system(72), 204) = Dword(-1);
                return 1;
            }
    return 0;
}
// The case-9 handler supplies this state.
void send_client_snapshot(void *owner) {
    void *car = packets::vehicle(at<std::int16_t>(owner, 378));
    if (!car)
        return;
    alignas(4) Byte state[80]{};
    void *body = at<void *>(car, 1768);
    at<Dword>(state, 64) = at<Dword>(body, 20);
    const Dword angular_y = at<Dword>(body, 24);
    const Dword angular_z = at<Dword>(body, 28);
    at<Dword>(state, 72) = angular_z;
    at<Dword>(state, 68) = angular_y;
    at<Dword>(state, 52) = at<Dword>(body, 8);
    const Dword velocity_y = at<Dword>(body, 12);
    const Dword velocity_z = at<Dword>(body, 16);
    at<Dword>(state, 60) = velocity_z;
    at<Dword>(state, 56) = velocity_y;
    for (unsigned word = 0; word < 9; ++word)
        at<Dword>(state, 16 + word * 4) = at<Dword>(body, 692 + word * 4);
    at<Dword>(state, 4) = at<Dword>(body, 680);
    const Dword position_y = at<Dword>(body, 684);
    const Dword position_z = at<Dword>(body, 688);
    at<Dword>(state, 12) = position_z;
    at<Dword>(state, 0) = game::race_ticks;
    Dword flags = at<Byte>(car, 1577) & 1;
    at<Dword>(state, 8) = position_y;
    flags |= at<Byte>(car, 1581) ? 2 : 0;
    flags |= at<Dword>(body, 732) ? 4 : 0;
    flags |= (flags & 4) << 1;
    if (at<Dword>(system(72), 152) == 1) {
        void *participant = application::pending::race_participant_owner_view(
            system(92), Dword(at<std::int16_t>(owner, 378)));
        if (participant && (race_pending(static_cast<Dword *>(participant)) & 255)) {
            at<Dword>(body, 732) = 90;
            flags &= ~8u;
            select_spectator_vehicle(owner);
        }
    }
    flags ^= (flags ^ (Dword(at<Byte>(system(36), 48)) << 8)) & 16128;
    at<Dword>(state, 76) = flags;
    alignas(4) Byte packed[36];
    pack_client_car(state, packed);
    send_client_car(owner, packed);
}
} // namespace nfs::network::session
