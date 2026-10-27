#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
namespace nfs::game::pending {
// Preserve the order, signed deadline, byte mask and early return.
void step_systems(Dword frame) {
    using namespace runtime;
    const Byte full_step = Byte(frame) & 1;
    const Dword step_flags = (frame & 4294967040u) | full_step;
    stepping = 1;
    if (!race_started) {
        if (!start_deadline) {
            if (!network::session_active())
                start_deadline = graphics::unresolved::camera_state + countdown_delay;
        } else if (static_cast<int>(start_deadline) <
                   static_cast<int>(graphics::unresolved::camera_state))
            race_started = count_race_ticks = 1;
    }
    for (int index = 0; index < step_list_count; ++index)
        events::dispatch(step_lists[index]);
    if (!at<Dword>(system(72), 140))
        runtime::pending::advance_track(system(36), step_flags);
    else
        runtime::pending::advance_replay_track_entry(system(36), step_flags);
    runtime::pending::update_network_entry(system(104));
    if (at<Dword>(system(108), 44) && at<Dword>(system(108), 48)) {
        if (race_started)
            ++started_frames;
        invoke(at<void *>(system(92), 4), 0);
        stepping = 0;
        return;
    }
    if (!network::session_active() || at<std::int16_t>(system(104), 10) >= 4) {
        if (full_step) {
            runtime::pending::prepare_start_line_entry(system(136));
            Dword *ai = system(76);
            runtime::pending::prepare_ai(ai);
            runtime::pending::prepare_vehicle_states();
            runtime::pending::prepare_cars(system(68));
            runtime::pending::update_ai_contacts(ai + 84208 / 4,
                                                          at<Dword>(system(72), 168));
        }
        runtime::pending::step_vehicle_physics(full_step);
        runtime::pending::finish_ai(system(76));
        if (full_step) {
            runtime::pending::finish_cars(system(68));
            runtime::pending::update_track_collision(track_collision);
            if (network::graphics_enabled)
                kernel::cooperate(0);
            runtime::pending::update_race_audio();
            runtime::pending::update_car_environment();
        }
    }
    runtime::pending::update_particles(system(64), step_flags);
    for (void *service : tick_services)
        if (service)
            invoke(service, 1);
    if (race_started)
        ++started_frames;
    invoke(at<void *>(system(92), 4), 0);
    stepping = 0;
    kernel::cooperate(0);
}
} // namespace nfs::game::pending
