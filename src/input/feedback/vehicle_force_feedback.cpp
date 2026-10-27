#include "vehicle_feedback_runtime.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
namespace nfs::services {
using game::runtime::at;
using game::runtime::bytes;
namespace {
long double limit_force(long double value, float maximum) {
    if (value < 0.0f)
        return 0.0f;
    if (maximum < value)
        return maximum;
    return value;
}
int force_integer(long double value) {
    return std::int32_t(kernel::Dword(game::runtime::hardware::truncate_integer_native80(&value)));
}
} // namespace

void set_steering_force(void *owner, float strength, int direction) {
    const int effect = at<int>(owner, 200);
    if (effect < 0)
        return;
    if (!(strength > 0)) {
        input::stop_effect(effect);
        return;
    }
    const int gain =
        force_integer(limit_force(static_cast<long double>(strength) * 100.f, 100.f));
    input::update_effect(effect, 0, nullptr, nullptr, &gain, nullptr, &direction, nullptr);
    input::start_effect(effect);
}

void set_impact_force(void *owner, float strength, int direction) {
    const int effect = at<int>(owner, 8);
    if (effect < 0)
        return;
    if (!(strength > 0)) {
        input::stop_effect(effect);
        return;
    }
    if (strength > 1)
        strength = 1;
    const long double amount = strength;
    const int gain = force_integer(limit_force(amount * 100.f, 100.f));
    at<kernel::Dword>(owner, 16) = 3000u - kernel::Dword(force_integer(amount * -7000.f));
    at<int>(owner, 28) = force_integer(amount * 1000.f);
    at<int>(owner, 24) = force_integer(amount * 18000.f);
    at<int>(owner, 20) = force_integer(amount * 10000.f);
    at<kernel::Dword>(owner, 64) = kernel::Dword(-1);
    if (direction < 0)
        direction = 0;
    else if (direction > 360)
        direction = 360;
    input::update_effect(effect, 0, bytes(owner) + 16, &at<kernel::Dword>(owner, 64), &gain,
                         nullptr, &direction, nullptr);
    input::start_effect(at<int>(owner, 8));
}

void set_lateral_force(void *owner, float strength, Byte positive) {
    const int effect = at<int>(owner, 104);
    if (effect < 0)
        return;
    if (!(strength > 0)) {
        input::stop_effect(effect);
        return;
    }
    const long double amount = strength;
    kernel::Dword duration =
        kernel::Dword(force_integer(limit_force(amount * 1000.f + 500.f, 1000.f))) * 3000u;
    const int gain = force_integer(limit_force(amount * 100.f, 100.f));
    const int direction = positive ? 315 : 45;
    input::update_effect(effect, 0, nullptr, &duration, &gain, nullptr, &direction, nullptr);
    input::start_effect(effect);
}
} // namespace nfs::services
