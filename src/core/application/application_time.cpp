#include "nfs/core/application/application.hpp"
#include "nfs/core/timing/timer.hpp"
namespace nfs::application {
// Preserve 32-bit wraparound before the signed comparison.
void wait_ticks(int duration) {
    const Dword start = kernel::timer_count();
    Dword now = kernel::timer_count();
    while (static_cast<std::int32_t>(now - (start + Dword(duration))) < 0) {
        kernel::cooperate(0);
        now = kernel::timer_count();
    }
}

Dword tick_frequency() {
    return kernel::timer_frequency;
}
} // namespace nfs::application
