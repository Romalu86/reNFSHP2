#include "frontend_race_loading.hpp"
#include "frontend_transition_math.hpp"
#include "../../audio/vehicles/vehicle_audio_progress_math.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstring>

namespace nfs::application::frontend::transition_math {

bool replay_available(std::uint32_t tick_bits) {
    std::int32_t ticks;
    std::memcpy(&ticks, &tick_bits, sizeof ticks);
    const auto seconds = static_cast<long double>(ticks) * 0.01666666753590106964111328125f;
    return seconds > 30.0L;
}

void advance_loading_screen(RaceLoadingScreen *screen) {
    const float completed = game::audio::vehicle::progress_math::loading_completion_f32();
    if (completed > screen->completed) {
        auto value = static_cast<long double>(completed);
        if (value < 0.0L)
            value = 0.0L;
        else if (value > 1.0L)
            value = 1.0L;
        const auto width = static_cast<long double>(screen->full_width) * value;
        const Dword width_bits = Dword(game::runtime::hardware::truncate_integer_native80(&width));
        int stored_width;
        std::memcpy(&stored_width, &width_bits, sizeof stored_width);
        screen->completed = float(value);
        screen->progress->bounds.width = stored_width;
    }
}
}
