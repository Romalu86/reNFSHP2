#include "ai_path_frame_x87.hpp"
#include "../population/ai_population_runtime.hpp"
#include <cfloat>
#include <cmath>
namespace nfs::game::runtime::ai::lifecycle::pending {
using kernel::numeric::Extended;

Extended path_node_distance(const float *node, const float *position) {
    alignas(4) unsigned char retained[10];
    path_frame::node_distance_native80(node, position, retained);
    return Extended::from_native80(retained, kernel::numeric::current_format());
}

void closest_section_sample(void *section, const float *position, int *sample, float *distance) {
    path_frame::closest_sample(section, position, sample, distance);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
