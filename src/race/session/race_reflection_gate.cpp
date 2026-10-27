#include "race_frame_draw.hpp"
#include "nfs/core/timing/frame_clock.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
namespace nfs::game::world::frame {
// Every saved consumer uses AL only.
Byte reflection_camera_enabled(void *director) {
    const int first_mode = at<int>(director, 312 + at<Dword>(director, 356) * 4);
    const int mode = at<int>(director, 312 + at<Dword>(director, 356) * 4);
    if (at<Byte>(director, 172) || (mode != 1 && mode != 0 && mode != 2 && mode != 3))
        return 0;
    return !at<Byte>(director, 164) && !at<Byte>(director, 165) &&
           !at<Byte>(director, 166) && first_mode != 2;
}
// Every saved consumer uses AL only; upper EAX is not a boolean.
Byte dynamic_reflection_enabled() {
    if (!at<Byte>(subsystem(32), 4) || at<Byte>(subsystem(48), 104) ||
        runtime::physics::split_screen() || frame_clock::paused || at<Dword>(subsystem(72), 140) ||
        !network::packets::rule_value(static_cast<Dword *>(subsystem(72)), 46))
        return 0;
    return tick_services[0] && reflection_camera_enabled(tick_services[0]);
}
} // namespace nfs::game::world::frame
