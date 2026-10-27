#include "../pursuit/police_chase_x87.hpp"
#include "../../ai/entities/ai_entity_runtime.hpp"
#include "../../physics/bodies/shape_body_runtime.hpp"
#include <cstring>
namespace nfs::game::runtime::ai::entities {

void update_spike_corners(void *entity) {
    if (void *body = invoke<void *>(interface_at(entity, 3), 1)) {
        const float x = at<float>(body, 120), z = at<float>(body, 128);
        const float corners[] = {-x, 0, z, x, 0, z, x, 0, -z, -x, 0, -z};
        matrices::transform_positions(4, corners,
                                      reinterpret_cast<const float *>(bytes(body) + 692),
                                      reinterpret_cast<const float *>(bytes(body) + 680),
                                      reinterpret_cast<float *>(bytes(entity) + 236), 12);
    }
}
// Only the low byte of the result is defined.
Byte settle_spike_body(void *entity) {
    return police_frame::settle_spike_body_retained(entity);
}
} // namespace nfs::game::runtime::ai::entities
