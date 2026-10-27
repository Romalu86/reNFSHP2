#include "ai_population_runtime.hpp"
#include <cstring>
namespace nfs::game::runtime::ai::lifecycle::pending {
Dword roadblock_minimum_cars = 1, roadblock_maximum_cars = 3;

void reset_police_chase(void *chase) {
    at<Dword>(chase, 72) = at<Dword>(chase, 68);
    at<Dword>(chase, 76) = Dword(-1);
    for (unsigned offset : {52u, 60u, 80u, 40u, 44u, 24u, 88u, 92u, 100u, 104u})
        at<Dword>(chase, offset) = 0;
    at<Byte>(chase, 108) = 0;
    if (at<int>(chase, 96) != -1) {
        release_population_location(bytes(state<void *>(7353292)) + (at<Dword>(chase, 96) << 6));
        at<Dword>(chase, 96) = Dword(-1);
    }
    if (void *car = at<void *>(chase, 48)) {
        at<Dword>(car, 408) = 0;
        at<void *>(chase, 48) = nullptr;
    }
}

void reset_path_population(Dword *controller) {
    load_spike_population(controller + 34);
}

void initialize_path_population(Dword *controller) {
    reset_police_chase(&state<Dword>(7352496));
    reset_police_chase(&state<Dword>(7352612));
    at<std::uint16_t>(controller, 88) = 0;
    state<Byte>(7352730) = 0;
    controller[37] = 0;
    invoke(bytes(controller) + 92, 1);
    initialize_population_locations(bytes(controller) + 120);
    std::memset(controller, 0, 8 * 4);
    controller[8] = controller[9] = controller[10] = controller[11] = 0;
    for (unsigned offset : {80u, 86u, 82u, 84u})
        at<std::uint16_t>(controller, offset) = 0;
    state<Byte>(7352728) = 1;
    const int difficulty = at<int>(system(72), 192);
    if (difficulty == 0) {
        roadblock_minimum_cars = 1;
        roadblock_maximum_cars = 3;
    } else if (difficulty == 1) {
        roadblock_minimum_cars = 2;
        roadblock_maximum_cars = 4;
    } else if (difficulty == 2) {
        roadblock_minimum_cars = 3;
        roadblock_maximum_cars = 5;
    }
}

void reset_path_controller(Dword *controller) {
    reset_police_chase(&state<Dword>(7352496));
    reset_police_chase(&state<Dword>(7352612));
    at<std::uint16_t>(controller, 88) = 0;
    state<Byte>(7352730) = 0;
    controller[37] = 0;
    dependencies::reset_path_reservations(bytes(controller) + 92);
    reset_helicopter_population(ai::collision_state());
    state<Dword>(7352444) = 0;
    network::world::Iterator iterator;
    network::world::construct_iterator(&iterator, state<Dword *>(7353312));
    for (Dword entity = network::world::first(&iterator); entity;
         entity = network::world::next(&iterator))
        invoke(reinterpret_cast<void *>(entity), 13);
    const unsigned racers = at<std::uint16_t>(controller, 80);
    for (unsigned i = 0; i < racers; ++i)
        if (controller[i])
            ai::pending::dispatch_vehicle_event(reinterpret_cast<Dword *>(controller[i]),
                                                         3);
    at<std::uint16_t>(controller, 84) = 0;
    Dword no_player_cop = 1;
    for (unsigned offset = 84164; offset < 84172; offset += 4)
        if (void *car = at<void *>(system(76), offset))
            if (ai::racer(car)) {
                no_player_cop = 0;
                break;
            }
    controller[28] = no_player_cop;
    clear_population_locations(bytes(controller) + 120);
    Dword count = at<std::uint16_t>(controller, 82);
    for (Dword *car = controller + 8; count; ++car, --count)
        if (*car)
            ai::pending::dispatch_vehicle_event(reinterpret_cast<Dword *>(*car), 3);
    reset_spike_population(controller + 34);
}

void release_path_population(Dword *controller) {
    reset_police_chase(&state<Dword>(7352496));
    reset_police_chase(&state<Dword>(7352612));
    at<std::uint16_t>(controller, 88) = 0;
    controller[37] = 0;
    std::memset(controller, 0, 8 * 4);
    controller[8] = controller[9] = controller[10] = controller[11] = 0;
    state<Byte>(7352730) = 0;
    dependencies::release_contact_registry(ai::collision_state());
    for (unsigned offset : {88u, 80u, 86u, 84u})
        at<std::uint16_t>(controller, offset) = 0;
    release_population_locations(bytes(controller) + 92);
    if (void *locations = at<void *>(controller, 132)) {
        kernel::memory_free(locations);
        controller[33] = 0;
    }
    controller[32] = 0;
    release_spike_population(controller + 34);
    state<Byte>(7352728) = 0;
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
