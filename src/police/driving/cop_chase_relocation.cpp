#include "../pursuit/police_chase_x87.hpp"
#include "cop_driving_runtime.hpp"
#include "../../math/vectors/retained_vector.hpp"
namespace nfs::game::runtime::ai::cars {
using namespace physics::car_frame;

Byte relocate_distant_chase_cop(void *entity) {
    return police_frame::relocate_distant_chase_cop_retained(entity);
}
} // namespace nfs::game::runtime::ai::cars
