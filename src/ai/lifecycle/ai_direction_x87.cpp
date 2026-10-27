#include "../driving/ai_route_planner.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstring>

namespace nfs::game::runtime::ai::entities::pending::direction_x87 {
namespace {

long double calculate_steering(void *entity, const float *target, void *controls) {
    invoke(entity, 30, target);
    void *view = interface_at(entity, 1, 320);
    void *body = invoke<void *>(view, 0);
    float *position = reinterpret_cast<float *>(bytes(view) + 4);
    if (body) {
        const Dword x = at<const volatile Dword>(body, 680);
        const auto negative_z = -static_cast<long double>(at<const volatile float>(body, 688));
        at<volatile Dword>(position, 0) = x;
        const Dword y = at<const volatile Dword>(body, 684);
        at<volatile float>(position, 8) = float(negative_z);
        at<volatile Dword>(position, 4) = y;
    } else {

        at<volatile Dword>(position, 0) = 0;
        at<volatile Dword>(position, 4) = 0;
        at<volatile Dword>(position, 8) = 0;
    }
    const float position_x = position[0];
    const float x = float(static_cast<long double>(target[0]) - position_x);
    const float position_z = position[2];
    const auto z = static_cast<long double>(target[2]) - position_z;
    if (x == 0.0f && z == 0.0L) {
        at<Byte>(controls, 4) = 0;
        return 0.0L;
    }
    const float negative_z = float(-z);
    const auto heading_difference = runtime::turn_direction_retained(x, negative_z) -
                                    at<float>(entity, 388);
    float angle = float(heading_difference);
    if (heading_difference >= 0.5L)
        angle = float(static_cast<long double>(angle) - 1.0f);
    else if (angle < -0.5f)
        angle = float(static_cast<long double>(angle) + 1.0f);
    auto result = static_cast<long double>(angle);
    if (invoke<long double>(entity, 22) < 0.0L)
        result = -result;
    auto scaled = 60.0L * result;
    if (scaled < -0.5L)
        scaled = -0.5L;
    else if (scaled > 0.5L)
        scaled = 0.5L;
    int steering = int(Dword(static_cast<std::int64_t>(scaled * 254.0f)));
    if (steering < -127)
        steering = -127;
    else if (steering > 127)
        steering = 127;
    at<std::int8_t>(controls, 4) = std::int8_t(steering);
    return result;
}
} // namespace

void direction_turns_native80(float x, float z, void *output) {
    const long double result = runtime::turn_direction_retained(x, z);
    std::memcpy(output, &result, 10);
}

void steer_toward_position_native80(void *entity, const float *target, void *controls,
                                  void *output) {
    const long double result = calculate_steering(entity, target, controls);
    std::memcpy(output, &result, 10);
}
} // namespace nfs::game::runtime::ai::entities::pending::direction_x87
