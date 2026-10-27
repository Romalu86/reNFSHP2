#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "../driving/ai_car_behavior.hpp"

namespace nfs::game::runtime::ai::pending {
// The pair cache keeps the squared distance and square root in separate F32 stores.
// The private pointer/F32 interface keeps the public MSVC ABI intact.
void store_pair_distances(float *pair, const float *position, const float *other_position) {
    const auto retained = hardware::distance_squared(position, other_position);
    pair[0] = float(retained);
    // The square root uses its own rounded squared-distance value.
    const float squared = float(retained);
    const float distance = float(__builtin_sqrtl(static_cast<long double>(squared)));
    pair[1] = distance;
}
} // namespace nfs::game::runtime::ai::pending

namespace nfs::game::runtime::ai::cars {
float local_player_distance(const float *position) {
    float nearest = float_word(2139095039);
    void *population = system(76);
    for (unsigned player = 0; player < 2; ++player) {
        if (void *entity = at<void *>(population, 84164 + player * 4)) {
            const auto distance =
                hardware::distance_squared(position, invoke<const float *>(entity, 0));
            if (distance < static_cast<long double>(nearest))
                nearest = float(distance);
        }
    }
    if (nearest == float_word(2139095039))
        return nearest;
    return float(__builtin_sqrtl(static_cast<long double>(nearest)));
}

long double __fastcall perp_glue_speed_factor(void *entity, void *) {
    if (!at<void *>(entity, 664))
        return 1.0L;
    void *tuning = system(76);
    if (at<float>(tuning, 84588) == 0.f)
        return 0.0L;
    const auto retained_multiplier =
        static_cast<long double>(at<float>(tuning, 84584)) - 1.f;
    void *target = at<void *>(entity, 664);
    const float multiplier = float(retained_multiplier);
    const float *own_position = invoke<const float *>(entity, 0);
    const float *target_position = invoke<const float *>(target, 0);
    float delta[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        delta[axis] = float(static_cast<long double>(own_position[axis]) - target_position[axis]);
    const auto distance = hardware::vector_length(delta);
    auto ratio = (distance - at<float>(tuning, 84592)) /
                 static_cast<long double>(at<float>(tuning, 84588));
    if (ratio < 0.0L)
        ratio = 0.0L;
    else if (ratio > 1.0L)
        ratio = 1.0L;
    return ratio * multiplier + 1.f;
}
} // namespace nfs::game::runtime::ai::cars
