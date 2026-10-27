#include "race_legacy_effects.hpp"
#include <cstring>
#include <intrin.h>
static_assert(__LDBL_MANT_DIG__ == 64, "This translation unit requires native x87 intermediates");
namespace nfs::game::world::frame {

Dword *pack_effect_colour(Dword *value, Dword alpha, Dword red, Dword green, Dword blue) {
    *value = (((alpha << 8 | (red & 255)) << 8 | (green & 255)) << 8) | (blue & 255);
    return value;
}

float *assign_effect_vector(float *value, float x, float y, float z, float w) {
    __movsd(reinterpret_cast<unsigned long *>(value),
            reinterpret_cast<const unsigned long *>(&x), 1);
    __movsd(reinterpret_cast<unsigned long *>(value + 1),
            reinterpret_cast<const unsigned long *>(&y), 1);
    __movsd(reinterpret_cast<unsigned long *>(value + 2),
            reinterpret_cast<const unsigned long *>(&z), 1);
    __movsd(reinterpret_cast<unsigned long *>(value + 3),
            reinterpret_cast<const unsigned long *>(&w), 1);
    return value;
}
// The saved pointer return aliases the output argument.
float *subtract_effect_vector(const float *left, float *output, const float *right) {
    for (unsigned lane = 0; lane < 4; ++lane)
        output[lane] = float(static_cast<long double>(left[lane]) - right[lane]);
    return output;
}
} // namespace nfs::game::world::frame
