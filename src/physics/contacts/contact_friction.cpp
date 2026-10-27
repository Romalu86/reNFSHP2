#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::game::runtime::physics {
// 0=no tangent, 1=static friction, 2=sliding friction.
int contact_friction(const float *momentum, float normal_momentum, const float *normal,
                     const float *friction, float *output) {
    const float approach = -normal_momentum;
    const auto E = [](float value) { return static_cast<long double>(value); };
    zero_triplet(output);
    if (approach <= 0)
        return 0;
    const float ny = normal[1], nz = normal[2], vy = momentum[1], vz = momentum[2];
    const float z_product = float(E(approach) * nz);
    output[0] = float(E(approach) * normal[0] + momentum[0]);
    output[1] = float(E(approach) * ny + vy);
    output[2] = float(E(z_product) + vz);
    const auto tangent = hardware::normalize_collision(output);
    const float magnitude = float(tangent);
    if (tangent <= 0)
        return 0;
    // Native retains negated X while Y/Z pass through their explicit F32 stores.
    const auto tangent_x = E(output[0]) * -1.0f;
    output[0] = float(tangent_x);
    const float tangent_y = float(E(output[1]) * -1.0f);
    output[1] = tangent_y;
    const float tangent_z = float(E(output[2]) * -1.0f);
    output[2] = tangent_z;
    const float dynamic_limit = float(E(approach) * friction[1]);
    if (E(approach) * friction[0] < E(magnitude)) {
        output[0] = float(tangent_x * dynamic_limit);
        output[1] = float(E(tangent_y) * dynamic_limit);
        output[2] = float(E(tangent_z) * dynamic_limit);
        return 2;
    }
    output[0] = float(tangent_x * magnitude);
    output[1] = float(E(tangent_y) * magnitude);
    output[2] = float(E(tangent_z) * magnitude);
    return 1;
}
} // namespace nfs::game::runtime::physics
