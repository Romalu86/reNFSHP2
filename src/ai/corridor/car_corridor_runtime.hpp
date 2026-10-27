#pragma once
#include "../driving/ai_driver_runtime.hpp"
#include "../../core/lifecycle/kernel_values.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include <cmath>
#include <utility>
namespace nfs::game::runtime::ai::cars {
Dword corridor_restrict_side(void *, void *, float, float, float);
Dword corridor_restrict_behind(void *, void *, float, float, float);
Dword corridor_apply_car(void *, void *, float, float, void *);
Dword corridor_apply_body(void *, void *, float, void *);
Dword corridor_apply_spike(void *, void *, float);
const float *path_sample_position(void *, int);
int path_minimum_sample(void *, int);
inline int corridor_lane_index(const kernel::numeric::Extended &projection) {
    const auto format = kernel::numeric::current_format();
    const float shifted =
        float(projection + 0.5f);
    const float rounded = float(std::floor(double(shifted)));
    const auto integer = kernel::numeric::Extended(rounded, format).truncate_to_integer();
    const int lane = std::int32_t(Dword(integer) + 20);
    return audio::vehicle::clamp_value(lane, 0, 39);
}
inline int corridor_lane_index(long double projection) {
    return corridor_lane_index(kernel::numeric::Extended::from_double(
        double(projection), kernel::numeric::current_format()));
}
} // namespace nfs::game::runtime::ai::cars
