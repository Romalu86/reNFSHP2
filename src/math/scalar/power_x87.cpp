#include <cmath>

#pragma intrinsic(pow)

namespace nfs::game::runtime::physics::legacy {
// Preserve the x87 result after the existing CRT operation. The original CRT
// implementation remains a separate conformance obligation.
long double power_retained(double base, double exponent) {
    return std::pow(base, exponent);
}
}
