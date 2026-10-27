#include "nfs/physics/frame/simulation_runtime.hpp"
#include <cmath>
#include <limits>
namespace nfs::game::runtime {
int rounded_cell(float value) {
    const double rounded = std::nearbyint(static_cast<double>(value));
    if (!(rounded >= -2147483648.0 && rounded < 2147483648.0))
        return (std::numeric_limits<int>::min)();
    return static_cast<int>(rounded);
}
} // namespace nfs::game::runtime
