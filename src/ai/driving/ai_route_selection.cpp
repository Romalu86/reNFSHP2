#include "ai_path_frame_x87.hpp"
#include "../population/ai_population_runtime.hpp"
#include <cmath>
namespace nfs::game::runtime::ai::lifecycle::pending {
void closest_section_sample(void *, const float *, int *, float *);
// Only AL is consumed by the callers.
Byte point_in_route_quad(const float *point, const float *first, const float *second,
                         const float *third, const float *fourth) {
    return path_frame::point_in_quad(point, first, second, third, fourth);
}
// The temporary vector wrappers are inlined additions, not a matrix rotation.
void *path_section_containing_position(void *path, const float *position) {
    return path_frame::containing_section(path, position);
}

void construct_pursuit_route(void *graph, void *reference, Dword *sample, Dword *direction,
                             const float *heading, const float *position, void *path, Dword use_path,
                             Dword unused) {
    path_frame::construct_route(graph, reference, sample, direction, heading, position, path,
                                use_path, unused);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
