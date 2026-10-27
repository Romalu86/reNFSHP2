#include "../../ai/driving/ai_path_frame_x87.hpp"
#include "../../ai/entities/ai_entity_runtime.hpp"
#include <cfloat>
#include <cmath>
#include <cstring>
namespace nfs::game::runtime::ai::lifecycle::pending {
kernel::numeric::Extended path_node_distance(const float *, const float *);
void closest_section_sample(void *, const float *, int *, float *);
} // namespace nfs::game::runtime::ai::lifecycle::pending
namespace nfs::game::runtime::ai::entities {
float helicopter_fire_distance = 100.f;
namespace {
int nearest_section_node(void *section, const float *position) {
    return lifecycle::pending::path_frame::nearest_node(section, position);
}
} // namespace
namespace pending {

Byte update_path_sample(void *path, const float *position) {
    if (void *section = at<void *>(path, 24)) {
        at<int>(path, 16) = nearest_section_node(section, position);
        return 1;
    }
    return 0;
}

void select_helicopter_destination(void *entity) {
    path_frame::select_helicopter_destination(entity);
}
void plan_graph_route(void *, void *, Dword, const float *, void *, int, const float *, void *,
                      int *, float *);

Byte target_route_branch(void *entity, int *direction, float *distance, Dword *branch) {
    void *current = invoke<void *>(entity, 1);
    const Dword sample = invoke<Dword>(entity, 2);
    if (!current || int(sample) < 0 || !at<Dword>(entity, 440) || at<int>(entity, 444) < 0)
        return 0;
    Dword *owner = system(76);
    *branch = 0;
    const Dword group = invoke<Dword>(entity, 15);
    int endpoint;
    const float *position = invoke<const float *>(entity, 0);
    plan_graph_route(bytes(owner) + 4 + group * 42080, current, sample, position,
                     at<void *>(entity, 440), at<int>(entity, 444),
                     at<const float *>(entity, 436), bytes(entity) + 448, &endpoint, distance);
    if (current == at<void *>(entity, 440)) {
        *direction = 1;
        if (*distance < 0 || std::isnan(*distance)) {
            *distance = -*distance;
            *direction = -1;
        }
    } else {
        *direction = endpoint == at<int>(current, 20) ? -1 : 1;
        *branch =
            at<std::int16_t>(entity, 1648) ? at<Dword>(entity, 448) : at<Dword>(entity, 440);
    }
    return 1;
}
} // namespace pending
} // namespace nfs::game::runtime::ai::entities
