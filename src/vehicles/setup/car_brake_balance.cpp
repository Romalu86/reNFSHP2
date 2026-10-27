#include "car_initialization_runtime.hpp"
namespace nfs::game::world::vehicle_setup {

void initialize_brake_balance(void *car) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    auto I = [&](int value) { return Extended::integer(value, format); };
    const auto weight = E(at<float>(car, 520)) * 9.81000041961669921875f;
    const auto inverse_length = E(1) / E(at<float>(car, 528));
    at<float>(car, 252) = 1;
    const float rear =
        float(E(at<float>(car, 548)) * at<float>(car, 528) * inverse_length * weight);
    const auto front =
        (E(1) - at<float>(car, 548)) * at<float>(car, 528) * inverse_length * weight;
    const auto height = E(at<float>(car, 544)) * 0.90909087657928466796875f;
    const auto transfer = inverse_length * height * .81f;
    const float divisor = float(E(1) - transfer);
    const float ratio = float(transfer / E(divisor));
    const auto moved_weight = inverse_length * height * weight * .81f;
    const auto denominator = front * .81f / E(divisor) - (moved_weight + rear) * .81f * ratio;
    const auto numerator = (front - moved_weight) * .81f * ratio + E(rear) * .81f / E(divisor);

    const auto balance = denominator / numerator;
    const float balance_stored = float(balance);
    at<float>(car, 252) = balance_stored;
    const auto front_torque =
        brake_torque(at<float>(car, 1472), at<Dword>(car, 1468), at<int>(car, 1464), 1);
    const auto front_sidewall = I(at<int>(car, 916)) * .01f * (I(at<int>(car, 912)) * .001f);
    const auto front_radius =
        (front_sidewall + front_sidewall + I(at<int>(car, 920)) * .0254f) * .5f;
    const float front_capacity = float(front_torque / front_radius);
    const auto rear_torque =
        brake_torque(at<float>(car, 1488), at<Dword>(car, 1484), at<int>(car, 1480), 1);
    const auto rear_sidewall = I(at<int>(car, 1192)) * .01f * (I(at<int>(car, 1188)) * .001f);
    const auto rear_radius =
        (rear_sidewall + rear_sidewall + I(at<int>(car, 1196)) * .0254f) * .5f;
    auto torque_ratio = rear_torque / rear_radius / E(front_capacity);
    if (torque_ratio < E(.5f))
        torque_ratio = E(.5f);
    else if (E(1) < torque_ratio)
        torque_ratio = E(1);
    const auto corrected = E(balance_stored) / torque_ratio;
    at<float>(car, 252) = float(corrected);
    if (corrected < E(.1f))
        at<float>(car, 252) = .1f;
    else if (E(1) < corrected)
        at<float>(car, 252) = 1;
    else
        at<float>(car, 252) = float(corrected);
}
} // namespace nfs::game::world::vehicle_setup
