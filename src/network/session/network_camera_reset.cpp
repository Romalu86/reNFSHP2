#include "../vehicles/remote_car_runtime.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
namespace nfs::game::world::pending {

void reset_network_camera(void *network_game) {
    const int cars = at<int>(subsystem(72), 168);
    if (!at<Dword>(subsystem(112), 68) || at<std::int16_t>(network_game, 10) < 4 || cars < 1)
        return;
    Dword initial_tick = 0;
    void *viewed_car = at<void *>(tick_services[0], 300);
    if (!viewed_car) {
        for (int index = 0; index < cars; ++index) {
            void *car = at<void *>(subsystem(68), 20 + Dword(index) * 4);
            if (car && runtime::physics::car_is_remote(car)) {
                const Dword tick = network::snapshots::newest_tick(car);
                if (tick && (!initial_tick || tick < initial_tick))
                    initial_tick = tick;
            }
        }
    } else {
        // Polling loop: no added timeout, yield or replacement tick.
        do {
            initial_tick = network::snapshots::newest_tick(viewed_car);
            if (initial_tick)
                break;
        } while (at<volatile std::int16_t>(network_game, 8) > 3);
    }
    if (!initial_tick)
        initial_tick = 1;
    race_ticks = initial_tick;
    const Dword negative_tick = Dword(0) - initial_tick;
    frame_clock::elapsed = 0;
    graphics::unresolved::camera_state = 0;
    runtime::started_frames = 0;
    frame_clock::paused = 0;
    count_race_ticks = 0;
    runtime::race_started = 0;
    frame_clock::step = 1.f;
    start_deadline = negative_tick;
    if (static_cast<std::int32_t>(negative_tick) < 0) {
        runtime::race_started = 1;
        count_race_ticks = 1;
    }
    for (int index = 0; index < cars; ++index) {
        void *car = at<void *>(subsystem(68), 20 + Dword(index) * 4);
        if (car && runtime::physics::car_is_remote(car))
            network::snapshots::apply_reset_snapshot(race_ticks, car);
    }
}
} // namespace nfs::game::world::pending
