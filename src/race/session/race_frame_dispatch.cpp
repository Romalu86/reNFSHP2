#include "race_frame_draw.hpp"
#include "nfs/core/timing/frame_clock.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::application::frontend {
using namespace game::world;
static Dword sampled_frames;
static Dword sampled_tick;

void draw_race_frame() {
    if (at<Dword>(subsystem(108), 44) && at<Dword>(subsystem(108), 48))
        return;
    ++sampled_frames;
    if (std::int32_t(sampled_frames) >= 30) {
        using kernel::numeric::Extended;
        const auto format = kernel::numeric::current_format();
        const Dword negative_tick = Dword((Extended(frame_clock::elapsed, format) * -60.f).truncate_to_integer());
        const std::int32_t delta = std::int32_t(Dword(0) - sampled_tick - negative_tick);
        const auto elapsed_ticks = Extended::integer(delta, format);
        sampled_tick = Dword((Extended(frame_clock::elapsed, format) * 60.f).truncate_to_integer());
        if (Extended(0.f, format) < elapsed_ticks) {
            sampled_frames = 0;
            frame::measured_frame_rate = float(Extended(1800.f, format) / elapsed_ticks);
        }
    }
    frame::advance_vehicle_renderables(subsystem(52));
    const int players = network::packets::rule_value(static_cast<Dword *>(subsystem(72)), 13);
    for (int player = 0; player < players; ++player) {
        void *director = game::tick_services[player];
        game::world::pending::bind_frontend_camera(director);
        frame::draw_camera_frame(director);
    }
    graphics::previous_frame_tick = graphics::unresolved::camera_state;
}
} // namespace nfs::application::frontend
