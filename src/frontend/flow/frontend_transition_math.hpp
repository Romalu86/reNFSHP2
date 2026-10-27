#pragma once
#include <cstdint>

namespace nfs::application::frontend {
struct RaceLoadingScreen;

namespace transition_math {
bool replay_available(std::uint32_t ticks);
void advance_loading_screen(RaceLoadingScreen *screen);
}
}
