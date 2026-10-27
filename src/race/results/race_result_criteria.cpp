#include "../session/race_session.hpp"
#include <cmath>
#include <climits>
#include "../rules/race_rules_x87.hpp"
namespace nfs::application::pending {
namespace {
using race_rules_x87::retained;
using race_rules_x87::truncate;
struct ClockParts {
    int minutes, seconds, hundredths;
};
ClockParts split_clock(long double seconds, bool retain_sign) {
    const int minutes = int(truncate(seconds * 0.01666666753590106964111328125f));
    const auto fraction = seconds - static_cast<long double>(int(Dword(minutes) * 60));
    const float stored = float(fraction);
    const bool negative = retain_sign ? !(0.f <= fraction) : !(0.f <= stored);
    const float biased = float(retained(stored) + (negative ? 0.499998986721038818359375f : -0.499998986721038818359375f));
    const int whole = game::runtime::hardware::current_integer_f32(biased);
    const auto hundredths = (retained(stored) - static_cast<long double>(whole)) * 100.0f;
    const bool hundredths_negative = !(0.f <= hundredths);
    const float biased_hundredths =
        float(hundredths + (hundredths_negative ? 0.499998986721038818359375f : -0.499998986721038818359375f));
    const int sub = game::runtime::hardware::current_integer_f32(biased_hundredths);
    return {minutes, whole, sub};
}
} // namespace
// The first seconds value stays extended; the second is stored as binary32.
Byte participant_faster_clock(void *first, const void *second) {
    const int first_lap = final_lap();
    const int a = first_lap < 0 ? -1 : at<int>(first, 56 + Dword(first_lap) * 4);
    const int second_lap = final_lap();
    const int b = second_lap < 0 ? -1 : at<int>(second, 56 + Dword(second_lap) * 4);
    if (a == -1)
        return 0;
    if (b == -1)
        return 1;
    const auto left_seconds = static_cast<long double>(a) * 0.01666666753590106964111328125f;
    const float right_seconds = float(static_cast<long double>(b) * 0.01666666753590106964111328125f);
    const ClockParts left = split_clock(left_seconds, false),
                     right = split_clock(static_cast<long double>(right_seconds), true);
    if (left.minutes != right.minutes)
        return left.minutes < right.minutes;
    if (left.seconds != right.seconds)
        return left.seconds < right.seconds;
    if (left.hundredths != right.hundredths)
        return left.hundredths < right.hundredths;
    return at<Dword>(grid_entry(system(72), at<Dword>(first, 4)), 0) != 0;
}

int compare_participant_criteria(void *first, const void *second) {
    int priority = 0;
    for (unsigned offset = 40; offset > 0; offset -= 4) {
        void *race = at<void *>(system(92), 4);
        ++priority;
        switch (at<Dword>(race, offset)) {
        case 0:
            if (participant_faster_clock(first, second))
                return priority;
            break;
        case 1: {
            const int first_lap = final_lap();
            const int a = first_lap < 0 ? -1 : at<int>(first, 456 + Dword(first_lap) * 4);
            const int second_lap = final_lap();
            const int b = second_lap < 0 ? -1 : at<int>(second, 456 + Dword(second_lap) * 4);
            if (a != -1 && (b == -1 || a < b))
                return priority;
            break;
        }
        case 2: {
            const int a = at<int>(first, 880), b = at<int>(second, 880);
            if ((a == -1 && b != -1) || (a != -1 && b != -1 && b < a))
                return priority;
            break;
        }
        case 3: {
            const int a = at<int>(first, 884), b = at<int>(second, 884);
            if ((a == -1 && b != -1) || (a != -1 && b != -1 && b < a))
                return priority;
            break;
        }
        case 4:
            if (at<int>(first, 28) < at<int>(second, 28))
                return priority;
            break;
        case 5:
            if (at<int>(first, 8) != -1 &&
                (at<int>(second, 8) == -1 || at<int>(first, 8) < at<int>(second, 8)))
                return priority;
            break;
        case 6:
            if (at<Byte>(second, 940) && !at<Byte>(first, 940))
                return priority;
            break;
        case 7:
            if (at<Byte>(second, 942) && !at<Byte>(first, 942))
                return priority;
            break;
        case 8:
            if (at<int>(second, 12) < at<int>(first, 12))
                return priority;
            if (at<int>(second, 12) <= at<int>(first, 12)) {
                network::packets::pending::find_race_vehicle(at<int>(first, 4));
                network::packets::pending::find_race_vehicle(at<int>(second, 4));
            }
            break;
        }
    }
    return 20;
}
} // namespace nfs::application::pending
