#include "retained_transcendentals.hpp"
#include <cmath>

#pragma intrinsic(sin, tan, atan, fmod)

namespace nfs::kernel::numeric::hardware {

// This isolated MSVC owner uses the x86 intrinsic register convention.
// It contains one retained input and one operation, with no binary64
// argument slot or second call that could spill the first input.
long double sine_retained(const void *argument) {
    return std::sin(read_retained_argument(argument));
}

long double tangent_retained(const void *argument) {
    return std::tan(read_retained_argument(argument));
}

long double arctangent_retained(const void *argument) {
    return std::atan(read_retained_argument(argument));
}

long double unit_remainder_retained(const void *argument) {
    return std::fmod(read_retained_argument(argument), 1.0L);
}

} // namespace nfs::kernel::numeric::hardware
