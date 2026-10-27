#include "race_session.hpp"
#include "../rules/race_rules_x87.hpp"
namespace nfs::application::pending {

int __fastcall evaluate_race_medal(void *race, void *, const int *thresholds, void *record,
                                   Byte cop) {
    void *progress = active_event_progress(system(72));
    const int lap = final_lap();
    const int *first = nullptr, *second = nullptr;
    float first_value, second_value;
    bool first_lower = true, second_lower = true;
    auto time_value = [&](int ticks) {
        const Dword scaled = race_rules_x87::truncate(
            static_cast<long double>(ticks) * 0.01666666753590106964111328125f * 100.0f);
        return float(static_cast<long double>(std::int32_t(scaled)) * 0.00999999977648258209228515625f);
    };
    if (thresholds[3] != -1) {
        first = thresholds + 3;
        first_value = time_value(at<int>(record, 8));
    }
    const Dword event = at<Dword>(system(72), 152);
    if (event != 1 && event != 2 && thresholds[0] != -1) {
        if (!first) {
            first = thresholds;
            first_value = float(static_cast<long double>(at<int>(record, cop ? 28 : 32)));
            if (cop)
                first_lower = false;
        } else {
            second = thresholds;
            second_value = float(static_cast<long double>(at<int>(record, 32)));
            second_lower = !cop;
        }
    }
    if (thresholds[6] != -1) {
        const int ticks = lap < 0 ? -1 : at<int>(record, 56 + Dword(lap) * 4);
        if (!first) {
            first = thresholds + 6;
            first_value = time_value(ticks);
            if (event == 12)
                first_lower = false;
        } else {
            second = thresholds + 6;
            second_value = time_value(ticks);
            if (event == 12)
                second_lower = false;
        }
    }
    if (thresholds[9] != -1) {
        const int position = at<int>(race, 34392);
        if (position < 0)
            return -1;
        const float value = float(static_cast<long double>(position) + 1.0f);
        if (!first) {
            first = thresholds + 9;
            first_value = value;
        } else {
            second = thresholds + 9;
            second_value = value;
        }
    }
    if (thresholds[12] != -1) {
        const float position = float(static_cast<long double>(at<int>(race, 34392)));
        if (position < 0 || position > 7)
            return -1;
        const float value = float(static_cast<long double>(position) + 1.0f);
        if (!first) {
            first = thresholds + 12;
            first_value = value;
        } else {
            second = thresholds + 12;
            second_value = value;
        }
    }
    if (thresholds[15] != -1) {
        const float value = float(static_cast<long double>(event_points(progress, 0)));
        if (!first) {
            first = thresholds + 15;
            first_value = value;
            first_lower = false;
        } else {
            second = thresholds + 15;
            second_value = value;
            second_lower = false;
        }
    }
    if (!first)
        return -1;
    auto meets = [&](int threshold, float value, bool lower) {
        const long double limit = static_cast<long double>(threshold), metric(value);
        // The FCOM branches include the unordered condition in both predicates.
        return lower ? !(limit < metric) : !(metric < limit);
    };
    for (int medal = 2; medal >= 0; --medal)
        if (meets(first[medal], first_value, first_lower) &&
            (!second || meets(second[medal], second_value, second_lower)))
            return medal;
    return -1;
}
} // namespace nfs::application::pending
