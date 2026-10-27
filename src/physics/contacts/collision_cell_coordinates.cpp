#include "nfs/physics/frame/simulation_runtime.hpp"
static_assert(__LDBL_MANT_DIG__ == 64, "This translation unit requires native x87 intermediates");
namespace nfs::game::runtime::physics {
// FSTP float precedes FISTP for each coordinate.
void cell_coordinates(const float *position, int *x, int *z) {
    const int x_coordinate =
        rounded_cell(float(static_cast<long double>(position[0]) * 0.02f + 32.0f));
    const auto scaled_z = static_cast<long double>(position[2]) * 0.02f;
    *x = x_coordinate;
    const int z_coordinate = rounded_cell(float(scaled_z + 32.0f));
    const int x_before_z_store = *x;
    *z = z_coordinate;
    // The two lower-bound decisions retain the values captured before output aliasing.
    if (x_before_z_store < 0)
        *x = 0;
    if (z_coordinate < 0)
        *z = 0;
    if (*x >= 64)
        *x = 63;
    if (*z >= 64)
        *z = 63;
}
} // namespace nfs::game::runtime::physics
