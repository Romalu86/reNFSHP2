#include "vehicle_feedback_runtime.hpp"
#include "nfs/input/controllers/controller_resources.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include <cstring>
namespace nfs::game::runtime::physics::contacts_pending {

void update_ground_effects(void *car) {
    auto retained = [](float value) { return static_cast<long double>(value); };
    auto signed_value = [](Dword value) {
        return static_cast<long double>(std::int32_t(value));
    };
    auto rule = [&](Dword value) { return network::packets::rule_value(system(72), value); };
    auto integer = [](long double value) {
        return std::int32_t(Dword(hardware::truncate_integer_native80(&value)));
    };
    auto clamp_direction = [&](long double value) {
        if (value < retained(45))
            return retained(45);
        if (retained(315) < value)
            return retained(315);
        return value;
    };
    if (at<int>(car, 1776) != -1 && rule(40) && !at<Dword>(system(72), 140)) {
        const int first = at<int>(car, 7996), second = at<int>(car, 8000);
        const int maximum = first > second ? first : second;
        controllers::pending::set_rumble(0, std::uint16_t(maximum), std::uint16_t(maximum));
        int impact_direction = integer((retained(1) - at<float>(car, 1604)) * 180.f);
        void *body = at<void *>(car, 1768);
        if (at<float>(body, 608) != 0 && at<float>(body, 660) != 0) {
            auto ratio = retained(at<float>(body, 608)) / retained(at<float>(body, 660)) * .05f;
            volatile float strength = ratio < retained(0) ? 0.f : retained(1) < ratio ? 1.f : float(ratio);
            strength = float(signed_value(rule(53)) * strength * .01f);
            const Dword scale = rule(55);
            strength = float(signed_value(scale) * strength * .01f);
            float direction[3];
            std::memcpy(direction, bytes(body) + 612, sizeof(direction));
            float local[3] = {0, 0, 0};
            matrices::transform_vectors(1, direction, &at<float>(body, 72), local);
            const Byte positive = local[0] > 0;
            services::set_lateral_force(system(144), strength, positive);
            impact_direction = positive ? 45 : 315;
        }
        if (!at<Byte>(car, 2204) && !at<Byte>(car, 2500))
            services::set_steering_force(system(144), 0, 180);
        else {
            const Dword gain = rule(53);
            volatile float strength =
                float((retained(at<float>(car, 2488)) + at<float>(car, 2192)) * .5f * signed_value(gain) * .01f);
            const auto scaled = signed_value(rule(57)) * strength * .01f * .005f;
            strength = float(scaled);
            if (scaled < retained(0))
                strength = -strength;
            auto steer = (retained(at<float>(car, 2264)) + at<float>(car, 1968)) * .5f;
            if (steer < retained(-.5f))
                steer = retained(-.5f);
            else if (retained(.5f) < steer)
                steer = retained(.5f);
            auto magnitude = steer < retained(0) ? -steer : steer;
            long double direction = retained(180);
            if (!(magnitude < retained(0.0333333350718021392822265625f))) {
                steer = steer * -1.f;
                magnitude = steer < retained(0) ? -steer : steer;
                strength = float(magnitude * strength * 29.9999980926513671875f);
                direction = steer * 360.f + 180.f;
                impact_direction = integer(clamp_direction(direction));
                direction = clamp_direction(direction);
            }
            services::set_steering_force(system(144), strength, integer(direction));
        }
        const int impact_first = at<int>(car, 7996), impact_second = at<int>(car, 8000);
        if (!impact_first && !impact_second)
            services::set_impact_force(system(144), 0, 180);
        else {
            const int impact_maximum = impact_first > impact_second ? impact_first : impact_second;
            services::set_impact_force(system(144), float(signed_value(Dword(impact_maximum)) * 0.00001525902189314365386962890625f),
                                       impact_direction);
        }
    }
    at<Dword>(car, 7996) = 0;
    at<Dword>(car, 8000) = 0;
}
} // namespace nfs::game::runtime::physics::contacts_pending
