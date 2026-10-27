#include "../../graphics/vehicles/car_wheel_effects_runtime.hpp"
#include "../motion/world_motion_x87.hpp"
namespace nfs::game::runtime::physics::contacts_pending::x87 {
namespace {

long double wheel_slip_value(void *car, int index) {
    auto E = [](float value) { return static_cast<long double>(value); };
    void *wheel = bytes(car) + 1948 + Dword(index) * 296;
    if (!at<Byte>(wheel, 256))
        return E(0);
    float angle = float(E(at<float>(wheel, 16)) * 6.283185482025146484375f);
    const float cosine = float(runtime::actor_cosine_retained(angle));
    angle = float(E(at<float>(wheel, 16)) * 6.283185482025146484375f);
    const float sine = float(runtime::actor_sine_retained(angle));
    const float rolling = float(E(at<float>(wheel, 216)) * at<float>(wheel, 156));
    const float yaw = float(E(at<float>(car, 3200)) * 6.283185482025146484375f);
    long double side = E(0), longitudinal = E(0);
    if (index < 2) {
        const float lateral = float(E(yaw) * at<float>(car, 112) + at<float>(car, 3160));
        const auto correction = E(at<float>(car, 532)) * .5f * yaw;
        const float speed = float(index == 1 ? E(at<float>(car, 3168)) - correction
                                             : correction + at<float>(car, 3168));
        side = E(lateral) * cosine - E(speed) * sine;
        longitudinal = (E(rolling) - E(speed) * cosine) - E(lateral) * sine;
    } else {
        const float half_track = float(E(at<float>(car, 536)) * .5f);
        side = E(at<float>(car, 3160)) - E(yaw) * at<float>(car, 116);
        const auto correction = E(half_track) * yaw;
        const float speed = float(index == 2 ? E(at<float>(car, 3168)) - correction
                                             : correction + at<float>(car, 3168));
        longitudinal = E(rolling) - speed;
    }
    const float square = float(longitudinal * longitudinal + side * side);
    const float slip = float(__builtin_sqrtl(E(square)));
    if (!(slip > 1))
        return E(0);
    const auto speed_x = E(at<float>(car, 3160)), speed_z = E(at<float>(car, 3168));
    const float speed_square = float(speed_x * speed_x + speed_z * speed_z);
    const float speed = float(__builtin_sqrtl(E(speed_square)));
    auto denominator = E(speed) * .5f;
    if (denominator < E(20))
        denominator = E(20);
    auto strength = E(slip) / denominator;
    if (strength < E(0))
        strength = E(0);
    else if (E(1) < strength)
        strength = E(1);
    float amount = float(strength * strength);
    const auto average = (E(at<float>(car, 3068)) + at<float>(car, 2772) + at<float>(car, 2476) +
                          at<float>(car, 2180)) *
                         .25f;
    if (average == 0.0L)
        return E(0);
    auto deflection = E(at<float>(wheel, 32)) + at<float>(wheel, 32);
    if (deflection < E(0))
        deflection = -deflection;
    if (E(amount) < deflection)
        amount = float(deflection);
    strength = (E(at<float>(wheel, 232)) / average) * amount;
    return strength < E(1) ? strength : E(1);
}
} // namespace
void wheel_slip_strength_native80(void *car, int wheel, void *output) {
    motion_x87::store_native80(wheel_slip_value(car, wheel), output);
}
} // namespace nfs::game::runtime::physics::contacts_pending::x87
