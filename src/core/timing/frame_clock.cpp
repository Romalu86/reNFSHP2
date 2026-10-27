#include "nfs/core/timing/frame_clock.hpp"
#include "nfs/core/timing/timer.hpp"
namespace nfs::frame_clock {

void initialize(){kernel::timer_add_handler(tick);}
}
