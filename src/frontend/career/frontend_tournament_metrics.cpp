#include "../screens/frontend_menu_roots.hpp"
#include "../../ai/driving/ai_car_spawn_runtime.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::application::frontend {
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
Dword tournament_sort_round;

Dword event_round_points(void *progress, Dword slot, Dword round) {
    const auto position = at<std::uint8_t>(progress, 308 + round * 24 + slot);
    return position > 7 ? 0 : 8 - position;
}

bool event_round_failed(void *progress, Dword slot, Dword round) {
    return at<std::uint8_t>(progress, 308 + round * 24 + slot) > 7;
}

std::uint16_t event_stage_time(void *progress, Dword slot) {
    return at<std::uint16_t>(progress, 500 + slot * 2);
}

std::int16_t event_best_lap_time(void *progress, Dword slot) {
    return at<std::int16_t>(progress, 548 + slot * 2);
}

Dword event_round_position(void *progress, Dword round, Dword slot) {
    return at<std::uint8_t>(progress, 308 + round * 24 + slot);
}

int __cdecl compare_tournament_round(const void *first, const void *second) {
    void *progress = pending::active_event_progress(system(72));
    const int a = int(
        event_round_position(progress, tournament_sort_round, *static_cast<const Dword *>(first)));
    const int b = int(
        event_round_position(progress, tournament_sort_round, *static_cast<const Dword *>(second)));
    if (a < 0)
        return b >= 0;
    if (b < 0)
        return -1;
    return a - b;
}

int __cdecl compare_grid_order(const void *first, const void *second) {
    void *rules = system(72);
    const Dword a = at<Dword>(pending::grid_entry(rules, *static_cast<const Dword *>(first)), 20);
    return int(at<Dword>(pending::grid_entry(rules, *static_cast<const Dword *>(second)), 20) - a);
}

const char *tournament_driver_name(Dword id) {
    using game::runtime::ai::cars::next_profile;
    Dword index = 0;
    void *profile = next_profile(index, 0);
    while (profile) {
        void *driver = invoke<void *>(profile, 2);
        if (driver) {
            // The profile vbtable is at +0; an entity's vbtable is at +4.
            auto *facet = bytes(driver) + at<const int *>(driver, 0)[1];
            if (invoke<Dword>(facet, 12) == id)
                return reinterpret_cast<const char *>(bytes(driver) + 116);
        }
        if (index >= game::runtime::ai::lifecycle::state<Dword>(7353148))
            break;
        ++index;
        profile = next_profile(index, 0);
    }
    static const char empty[4]{};
    return empty;
}

void format_tournament_time(int ticks, char *output) {
    constexpr float tick_seconds = 0.01666666753590106964111328125f;
    constexpr float rounding_bias = 0.499998986721038818359375f;
    const auto seconds = static_cast<long double>(ticks) * tick_seconds;
    const auto minute_value = static_cast<long double>(tick_seconds) * seconds;
    const Dword minute_bits = Dword(game::runtime::hardware::truncate_integer_native80(&minute_value));
    int minutes;
    std::memcpy(&minutes, &minute_bits, sizeof(minutes));
    const Dword elapsed_bits = minute_bits * 60;
    int elapsed_seconds;
    std::memcpy(&elapsed_seconds, &elapsed_bits, sizeof(elapsed_seconds));
    const float remainder = float(seconds - static_cast<long double>(elapsed_seconds));
    // Each bias result is stored as F32 before the current-rounding FISTP32.
    const float rounded = float(remainder >= 0.0f
                                    ? static_cast<long double>(remainder) - rounding_bias
                                    : static_cast<long double>(remainder) + rounding_bias);
    const int whole = game::runtime::hardware::current_integer_f32(rounded);
    const auto fraction =
        (static_cast<long double>(remainder) - static_cast<long double>(whole)) * 100.0f;
    const float rounded_fraction =
        float(fraction >= 0.0f ? fraction - rounding_bias : fraction + rounding_bias);
    const int hundredths = game::runtime::hardware::current_integer_f32(rounded_fraction);
    std::sprintf(output, "%02d:%02d.%02d", minutes, whole, hundredths);
}
} // namespace nfs::application::frontend
