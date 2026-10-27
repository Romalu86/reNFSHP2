#include "../session/race_session.hpp"
#include "race_profile_metrics.hpp"
#include "../../math/vectors/frame_x87_private.hpp"

namespace nfs::application::pending {
// The external float-return ABI keeps the hardware load even for a copy.
__declspec(noinline) float load_profile_speed(const volatile float *source) {
    return *source;
}

namespace {
long double retained_speed(const float *source) {
    return static_cast<long double>(load_profile_speed(source));
}

float select_speed(const float *saved, const float *current) {
    const auto candidate = retained_speed(current);
    const auto previous = retained_speed(saved);
    return load_profile_speed(previous < candidate ? current : saved);
}
}

void record_profile_track_speeds(void *profile, std::uint32_t track, float first, float second) {
    at<float>(profile, 5240 + track * 4) = load_profile_speed(&first);
    at<float>(profile, 6324 + track * 4) = load_profile_speed(&second);
}

void record_profile_car_speeds(void *profile, std::uint32_t car, float first, float second) {
    at<float>(profile, 5288 + car * 4) = load_profile_speed(&first);
    at<float>(profile, 6372 + car * 4) = load_profile_speed(&second);
}

namespace profile_metrics {
void select_speeds(const float *saved_maximum, const float *saved_average,
                   const float *maximum, const float *average, float *output) {
    output[1] = select_speed(saved_average, average);
    output[0] = select_speed(saved_maximum, maximum);
}

void update_maximum(float *saved, const float *maximum) {
    const auto candidate = retained_speed(maximum);
    const auto previous = retained_speed(saved);
    const auto selected = previous < candidate ? retained_speed(maximum) : retained_speed(saved);
    const auto previous_again = retained_speed(saved);
    const float stored_previous = float(previous_again);
    *saved = float(previous_again > selected ? static_cast<long double>(stored_previous) : selected);
}
}
}
