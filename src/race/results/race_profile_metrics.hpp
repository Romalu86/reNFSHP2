#pragma once
#include <cstdint>

namespace nfs::application::pending {
void record_profile_track_speeds(void *profile, std::uint32_t track, float first, float second);
void record_profile_car_speeds(void *profile, std::uint32_t car, float first, float second);

namespace profile_metrics {
// Output is maximum, average; the average is evaluated and stored first.
void select_speeds(const float *saved_maximum, const float *saved_average,
                   const float *maximum, const float *average, float *output);
void update_maximum(float *saved, const float *maximum);
}
}
