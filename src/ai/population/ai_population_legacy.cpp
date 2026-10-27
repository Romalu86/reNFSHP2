#include "ai_population_x87.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending::population_frame {
// Keep these coefficients in their original F32 representation.
const float chase_transition_multiplier = 0.f;
const float helicopter_scale = 0.829999983310699462890625f;
const float helicopter_attachment = 1.60000002384185791015625f;
const float helicopter_projectile_x = -0.18491999804973602294921875f;
const float helicopter_projectile_y = 0.291079998016357421875f;
const float helicopter_projectile_z = 6.751430034637451171875f;
std::uint32_t truncate_native80(const void *value) {
    return std::uint32_t(hardware::truncate_integer_native80(value));
}
}
