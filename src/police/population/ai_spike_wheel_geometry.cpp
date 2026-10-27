#include "../pursuit/police_chase_x87.hpp"
#include "../../ai/driving/ai_car_behavior.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "../../math/vectors/retained_vector.hpp"
namespace nfs::game::runtime::ai::cars {

Dword spike_wheel_hit_mask(void *spike, void *entity) {
    return police_frame::spike_wheel_hit_mask_retained(spike, entity);
}
} // namespace nfs::game::runtime::ai::cars
