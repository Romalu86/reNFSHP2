#include "nfs/core/timing/frame_clock.hpp"
static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Frame clock arithmetic requires the retained x87 provider");
namespace nfs::frame_clock {
kernel::Dword ticks=0;
std::uint8_t paused=0;
float elapsed=0,step=0;
namespace {
// The multiplier is read from a single-precision memory operand.
const volatile float tick_seconds = 0.01666666753590106964111328125f;
}
void __cdecl tick() {
    ++ticks;
    if ((ticks & 1u) && !paused) {
        const long double scaled_step = static_cast<long double>(step) * tick_seconds;
        elapsed = static_cast<float>(scaled_step + static_cast<long double>(elapsed));
    }
}
}
