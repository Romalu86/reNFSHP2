#include "nfs/core/frame/game_step.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/core/timing/frame_clock.hpp"
#include "nfs/input/directinput/directinput.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::game {

Dword set_device_snapshot(void *manager, Dword index, Dword state) {
    auto *bytes = static_cast<std::uint8_t *>(manager);
    if (index < 32 && bytes[1012 + index * 8]) {
        *reinterpret_cast<Dword *>(bytes + 1016 + index * 8) = state;
        return 1;
    }
    return 0;
}

void poll_input() {
    if (!graphics::device)
        return;
    keyboard_snapshot = reinterpret_cast<Dword>(input::keyboard_get(6));
    auto *manager = reinterpret_cast<std::uint8_t *>(lifecycle::manager->slots[100 / 4]);
    const Dword count = reinterpret_cast<Dword>(input::device_get(0, 1));
    for (Dword index = 0; index < count; ++index)
        if (index < 32 && manager[1012 + index * 8])
            set_device_snapshot(manager, index,
                                reinterpret_cast<Dword>(input::device_get(index, 6)));
}
// The multiply is retained until ftol or the explicit float store.
void advance() {
    kernel::cooperate(0);
    int budget = 15;
    if (frame_clock::paused)
        for (void *service : tick_services) {
            if (!service)
                continue;
            using Tick = void(__thiscall *)(void *);
            reinterpret_cast<const Tick *>(*static_cast<Dword *>(service))[1](service);
        }
    auto &ticks = graphics::unresolved::camera_state;
    const auto target_frame = [] {
        const auto format = kernel::numeric::current_format();
        const auto value = kernel::numeric::Extended(frame_clock::elapsed, format) * 60.f;
        return std::int32_t(Dword(value.truncate_to_integer()));
    };
    int target = target_frame();
    while (static_cast<int>(ticks) < target) {
        poll_input();
        simulation_time = float(kernel::numeric::Extended::integer(
                                    std::int32_t(ticks), kernel::numeric::current_format()) *
                                0.01666666753590106964111328125f);
        if (frame_clock::paused || stop_requested)
            return;
        pending::step_systems(ticks);
        if (!frame_clock::paused) {
            ++ticks;
            if (count_race_ticks)
                ++race_ticks;
        }
        if (!--budget)
            return;
        target = target_frame();
    }
}
} // namespace nfs::game
