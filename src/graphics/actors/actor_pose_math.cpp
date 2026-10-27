#include "actor_pose_math.hpp"

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Actor pose arithmetic requires the retained x87 format");

namespace nfs::game::world::actor_resources::pending {
namespace {
constexpr unsigned actor_ground_offset = 116;
constexpr unsigned actor_heading_x_offset = 136;
constexpr unsigned actor_heading_z_offset = 144;
const volatile float degrees_per_half_turn = 180.0f;
const volatile float radians_per_half_turn = 3.1415927410125732421875f;
}

void apply_actor_matrix_coefficients(float *matrix, const float *coefficients) {
    volatile float *output = matrix;
    const volatile float *factors = coefficients;
    const long double first_component = output[0];
    output[0] = static_cast<float>(first_component * factors[0]);
    for (unsigned index = 1; index < 16; ++index) {
        const long double factor = factors[index];
        const long double component = output[index];
        output[index] = static_cast<float>(factor * component);
    }
}

void add_actor_ground_offset(float *position, const void *actor) {
    volatile float *output = position;
    const long double height = output[1];
    const auto *offset = reinterpret_cast<const volatile float *>(
        static_cast<const char *>(actor) + actor_ground_offset);
    output[1] = static_cast<float>(height + *offset);
}

float actor_heading_degrees(const void *actor) {
    const char *base = static_cast<const char *>(actor);
    const std::uint32_t z_bits = *reinterpret_cast<const volatile std::uint32_t *>(
        base + actor_heading_z_offset);
    const std::uint32_t x_bits = *reinterpret_cast<const volatile std::uint32_t *>(
        base + actor_heading_x_offset);
    const volatile float heading = actor_heading_radians(x_bits, z_bits);
    const long double numerator = degrees_per_half_turn;
    const long double scale = numerator / radians_per_half_turn;
    const long double opposite_heading = -static_cast<long double>(heading);
    return static_cast<float>(scale * opposite_heading);
}

} // namespace nfs::game::world::actor_resources::pending
