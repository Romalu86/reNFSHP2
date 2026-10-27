#include "nfs/physics/frame/simulation_runtime.hpp"
#include <cmath>

#pragma intrinsic(sin, cos, tan, atan2, sqrt)

namespace nfs::game::runtime {
extern const float turn_radians;
extern const float turns_per_radian;

// The selected C++ intrinsic emits the native FSIN provider.
long double turn_sine(float angle) {
    return std::sin(static_cast<long double>(angle) * turn_radians);
}

// Return the retained x87 result without a binary64 spill.
long double turn_cosine(float angle) {
    return std::cos(static_cast<long double>(angle) * turn_radians);
}


long double turn_tangent_retained(float angle) {
    return std::tan(static_cast<long double>(angle) * turn_radians);
}


long double turn_direction_retained(float x, float z) {
    return std::atan2(static_cast<long double>(x), static_cast<long double>(z)) *
           turns_per_radian;
}


long double actor_sine_retained(float radians) {
    return std::sin(static_cast<long double>(radians));
}

long double actor_cosine_retained(float radians) {
    return std::cos(static_cast<long double>(radians));
}


// FSQRT feeds FPATAN directly and the result returns in ST0, without F64 spills.
long double network_rotation_angle_retained(float cosine) {
    return std::atan2(std::sqrt(1.0L - static_cast<long double>(cosine) * cosine),
                      static_cast<long double>(cosine));
}
} // namespace nfs::game::runtime
