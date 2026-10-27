#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include "../../math/vectors/retained_vector_providers.hpp"
namespace nfs::game::runtime {
kernel::numeric::Extended retained_distance_squared(const float *first, const float *second) {
    const auto format = kernel::numeric::current_format();
    unsigned char distance[10];
    hardware::store_distance_squared(first, second, distance);
    return kernel::numeric::Extended::from_native80(distance, format);
}
 
long double distance_squared(const float *first, const float *second) {
    return double(retained_distance_squared(first, second));
}
// Sequential writes are observable for partial overlap.
void add_vector(const float *first, const float *second, float *output) {
    output[0] = first[0] + second[0];
    output[1] = first[1] + second[1];
    output[2] = first[2] + second[2];
}
} // namespace nfs::game::runtime
