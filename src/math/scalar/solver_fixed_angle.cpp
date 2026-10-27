#include "nfs/physics/solver/solver_runtime.hpp"
namespace nfs::game::runtime::race_audio {
// Fixed-angle octant lookup, including the 64-bit division path.
int fixed_atan2(int vertical, int horizontal) {
    Dword denominator = Dword(horizontal), numerator = Dword(vertical), base = 0;
    bool reverse = false;
    if (horizontal < 0) {
        denominator = 0 - denominator;
        reverse = true;
        base = 512;
    }
    if (vertical < 0) {
        numerator = 0 - numerator;
        reverse = !reverse;
        base = 0 - base;
    }
    if (static_cast<int>(numerator) > static_cast<int>(denominator)) {
        Dword temporary = denominator;
        denominator = numerator;
        numerator = temporary;
        base += reverse ? Dword(-256) : 256;
        reverse = !reverse;
    }
    Dword angle;
    if (numerator == denominator)
        angle = 128;
    else {
        Dword index;
        if (numerator & 4286578688u) {
            std::uint64_t quotient = (std::uint64_t(numerator) << 32) / denominator;
            index = Dword(quotient) >> 24;
            if (quotient & 8388608)
                ++index;
        } else
            index = ((denominator >> 1) + (numerator << 8)) / denominator;
        angle = saved::atan_octant[index];
    }
    return static_cast<int>(reverse ? base - angle : base + angle);
}
} // namespace nfs::game::runtime::race_audio
