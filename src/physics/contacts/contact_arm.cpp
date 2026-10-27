#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::game::runtime::physics {
// The temporary polymorphic vector addition is scalarized;
// constructor unwind and the vector interface identity remain separate ABI obligations.
void contact_arm(void *body, const float *point, float *output) {
    float transformed[3];
    matrices::transform_vectors(1, reinterpret_cast<float *>(bytes(body) + 44),
                                reinterpret_cast<float *>(bytes(body) + 692), transformed);
    const float z = float(static_cast<long double>(transformed[2]) + at<float>(body, 688));
    const float y = float(static_cast<long double>(transformed[1]) + at<float>(body, 684));
    const float x = float(static_cast<long double>(transformed[0]) + at<float>(body, 680));
    output[0] = float(static_cast<long double>(point[0]) - x);
    output[1] = float(static_cast<long double>(point[1]) - y);
    output[2] = float(static_cast<long double>(point[2]) - z);
}
} // namespace nfs::game::runtime::physics
