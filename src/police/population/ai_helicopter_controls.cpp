#include "../pursuit/police_chase_x87.hpp"
#include "../../ai/entities/ai_entity_runtime.hpp"
#include <cmath>
namespace nfs::game::runtime::ai::entities::pending {

void apply_helicopter_controls(void *entity, float desired_speed, const float *target) {
    police_frame::apply_helicopter_controls_retained(entity, desired_speed, target);
}
} // namespace nfs::game::runtime::ai::entities::pending
