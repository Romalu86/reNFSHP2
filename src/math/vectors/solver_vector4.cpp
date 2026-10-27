#include "nfs/physics/solver/solver_runtime.hpp"
#include "retained_vector.hpp"
#include "retained_vector_providers.hpp"
namespace nfs::game::runtime {
// Each component is written before the next pair is read.
void add_vector4(const float *first, const float *second, float *output) {
    for (unsigned i = 0; i < 4; ++i)
        output[i] = float(static_cast<long double>(first[i]) + second[i]);
}

void subtract_vector4(const float *first, const float *second, float *output) {
    for (unsigned i = 0; i < 4; ++i)
        output[i] = float(static_cast<long double>(first[i]) - second[i]);
}
kernel::numeric::Extended retained_point_distance(const float *first, const float *second) {
    const auto format = kernel::numeric::current_format();
    unsigned char distance[10];
    hardware::store_point_distance(first, second, distance);
    return kernel::numeric::Extended::from_native80(distance, format);
}

long double point_distance(const float *first, const float *second) {
    return double(retained_point_distance(first, second));
}
} // namespace nfs::game::runtime
