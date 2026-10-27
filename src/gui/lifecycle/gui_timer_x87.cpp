#include "gui_timer_x87.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
#include "nfs/gui/lifecycle/gui_modal.hpp"
#include "nfs/core/timing/timer.hpp"
#include <cstring>

namespace nfs::gui {
extern Container timer_entries;
}

namespace nfs::gui::timer_x87 {
namespace {
// Both operands are signed DWORD FILD inputs.
long double seconds_value(int ticks) {
    if (!kernel::timer_frequency)
        return 0.0f;
    return static_cast<long double>(ticks) /
           static_cast<long double>(static_cast<int>(kernel::timer_frequency));
}
}

void ticks_to_seconds_native80(int ticks, void *output) {
    const long double value = seconds_value(ticks);
    std::memcpy(output, &value, 10);
}

// Timer callbacks may change the container and its count.
void tick_registered_timers() {
    for (Dword index = 0; index < timer_entries.count; ++index) {
        auto *timer = static_cast<Dword *>(events::container_value(&timer_entries, index));
        if (!timer)
            continue;
        const float period = reinterpret_cast<float *>(timer)[2];
        if ((period < 0.0f) != (period == 0.0f))
            continue;
        const long double now = seconds_value(static_cast<int>(kernel::timer_count()));
        const float previous = reinterpret_cast<float *>(timer)[4];
        if (previous != 0.0f) {
            const float next = static_cast<float>(static_cast<long double>(previous) +
                                                reinterpret_cast<float *>(timer)[2]);
            if (now >= static_cast<long double>(next)) {
                reinterpret_cast<float *>(timer)[4] = next;
                post_target(timer[1], 22, timer[3], 0);
            }
        } else {
            reinterpret_cast<float *>(timer)[4] = static_cast<float>(now);
        }
    }
}
}
