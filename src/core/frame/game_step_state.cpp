#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/core/timing/frame_clock.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
namespace nfs::game::runtime {
Dword *step_lists[100]{};
int step_list_count{};
Dword countdown_delay{}, started_frames{};
Byte race_started{}, stepping{}, reset_requested{}, reset_complete{}; // 1ED/1F8/1EC/1D4/1D6.
Dword *track_collision{};
float &network_step_scale = frame_clock::step;  
Byte &race_waiting = frame_clock::paused;       

void initialize_countdown_delay() {
    countdown_delay = 300;
}

void clear_step_lists() {
    step_list_count = 0;
    kernel::memory_zero(step_lists, 400);
}
// The caller is responsible for the capacity check.
void add_step_list(Dword *list) {
    step_lists[step_list_count++] = list;
    events::acquire(list);
}

void remove_step_list(Dword *list) {
    for (unsigned index = 0; index < 100; ++index)
        if (step_lists[index] == list) {
            --step_list_count;
            step_lists[index] = step_lists[step_list_count];
            step_lists[step_list_count] = nullptr;
            events::release(list);
            return;
        }
}
 
void reset_contact_scratch() {
}

void reset_step_state() {
    stop_requested = 0;
    reset_complete = phase_reset = reset_requested = phase_pending = phase_locked = 0;
    started_frames = 0;
    reset_contact_scratch();
    step_list_count = 0;
    kernel::memory_zero(step_lists, 400);
}
} // namespace nfs::game::runtime
 
NFS_KERNEL_INITIALIZER(startup_initialize_countdown_delay, ".CRT$XCU_NFS_006901E4",
                       nfs::game::runtime::initialize_countdown_delay);
