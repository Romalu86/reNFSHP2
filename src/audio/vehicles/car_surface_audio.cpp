#include "../../graphics/vehicles/car_wheel_effects_runtime.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::game::runtime::physics::contacts_pending {
// Receiver is the damage mask at car+1F10.
Byte wheel_is_punctured(void *damage, Byte wheel) {
    return (at<Byte>(damage, 0) & Byte(1u << (wheel & 31))) != 0;
}
namespace {
void queue_surface_sound_value(void *car, Dword kind, Dword channel, Dword material,
                               long double value, Dword extra) {
    if (at<int>(car, 3208) >= 8)
        return;
    if (value < 0.f)
        value = 0.f;
    else if (10.f < value)
        value = 10.f;
    value = value * 10.f;
    at<Dword>(car, 3212 + at<Dword>(car, 3208) * 20) = kind;
    at<Dword>(car, 3216 + at<Dword>(car, 3208) * 20) = channel;
    const volatile float sample = float(value * 65536.f);
    at<Dword>(car, (at<Dword>(car, 3208) + 161) * 20) = material;
    const int integer = hardware::current_integer_f32(sample);
    at<Dword>(car, 3224 + at<Dword>(car, 3208) * 20) = Dword(integer);
    at<Dword>(car, 3228 + at<Dword>(car, 3208) * 20) = extra;
    ++at<Dword>(car, 3208);
}
} // namespace

void queue_car_sound(void *car, Dword kind, Dword channel, Dword material, float strength,
                     Dword extra) {
    queue_surface_sound_value(
        car, kind, channel, material,
        static_cast<long double>(strength), extra);
}

void update_axle_surface_audio(void *car, Byte front, Dword first_surface, Dword second_surface) {
    auto retained_value = [](float value) { return static_cast<long double>(value); };
    auto rolling_sound = [&](Dword surface) {
        const Dword kind = at<Dword>(pending::surface_properties(int(surface)), 0);
        switch (kind) {
        case 4:
        case 23:
        case 25:
        case 29:
        case 30: {
            const Dword selected = at<Dword>(pending::surface_properties(int(surface)), 0);
            auto strength = retained_value(at<float>(car, 3168)) * 2.2369899749755859375f * 0.008333333767950534820556640625f;
            if (strength < 0.f)
                strength = strength * -1.f;
            queue_surface_sound_value(car, 6, 18, selected | 256, strength, 0);
            return true;
        }
        default:
            return false;
        }
    };
    if (!front && !rolling_sound(first_surface))
        rolling_sound(second_surface);
    const float first = at<float>(car, front ? 1700 : 1708);
    const float second = at<float>(car, front ? 1704 : 1712);
    // Front and rear intentionally have opposite saved material selectors.
    const bool use_first = front ? second <= first : first <= second;
    const Dword material = at<Dword>(
        pending::surface_properties(int(use_first ? first_surface : second_surface)), 0);
    const Byte first_wheel = front ? 0 : 2, second_wheel = front ? 1 : 3;
    void *damage = bytes(car) + 7952;
    if (!wheel_is_punctured(damage, first_wheel) && !wheel_is_punctured(damage, second_wheel)) {
        if (first != 0 || second != 0) {
            float strength;
            if (first != 0 && second != 0)
                strength = second < first ? first : second;
            else
                strength = first != 0 ? first : second;
            queue_car_sound(car, front ? 3 : 4, 18, material, strength, 0);
            at<Byte>(car, front ? 1692 : 1693) = 1;
        } else if (at<Byte>(car, front ? 1692 : 1693)) {
            at<Byte>(car, front ? 1692 : 1693) = 0;
        }
        return;
    }
    if (!front && (wheel_is_punctured(damage, 0) || wheel_is_punctured(damage, 1)))
        return;
    auto strength = retained_value(at<float>(car, 3168)) * 2.2369899749755859375f * 0.0333333350718021392822265625f;
    if (strength < 0.f)
        strength = strength * -1.f;
    if (1.f < strength)
        strength = 1.f;
    queue_surface_sound_value(car, front ? 3 : 4, 0, material, strength, 0);
}
} // namespace nfs::game::runtime::physics::contacts_pending
