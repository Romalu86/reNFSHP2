#pragma once
#include <cstdint>

namespace nfs::gui::timer_x87 {
// The public compatibility class stays within MSVC; only ten value bytes cross here.
void ticks_to_seconds_native80(int ticks, void *output);
void tick_registered_timers();
void dialog_tick(void *widget, std::uint32_t ticks);
void ticker_timer(void *ticker, std::uint32_t id);
void update_ticker(void *ticker, std::uint32_t ticks);
}
