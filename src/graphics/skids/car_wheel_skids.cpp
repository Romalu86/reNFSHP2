#include "../vehicles/car_wheel_effects_runtime.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::runtime::physics::contacts_pending {

void update_wheel_skid(void *car, int index) {
    if (!at<Byte>(system(140), 7)) {
        particles::lifecycle::release_wheel_skid(car, index);
        return;
    }
    const Dword wheel_offset = Dword(index) * 296;
    void *ground = bytes(car) + wheel_offset + 1996;
    if (!ground || !at<Dword>(ground, 24))
        return;
    const Byte material = at<Dword>(ground, 24) ? at<Byte>(ground, 78) & 31 : 0;
    const int style = at<int>(pending::surface_properties(material), 20);
    const float slip = at<float>(car, 1700 + Dword(index) * 4);
    const float threshold = at<float>(system(140), 8);
    if (threshold < 1 && style && at<Byte>(car, wheel_offset + 2204) &&
        (style == 3 || threshold <= slip)) {
        const auto format = kernel::numeric::current_format();
        using kernel::numeric::Extended;
        const float strength =
            float((Extended(slip, format) - threshold) / (Extended(1.f, format) - threshold));
        append_wheel_skid(car, index, style, strength);
    }
}

void append_wheel_skid(void *car, int index, int style, float strength) {
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    const Dword wheel_offset = Dword(index) * 296;
    void *ground = bytes(car) + wheel_offset + 1996;
    if (!ground || !at<Dword>(ground, 24))
        return;
    void *body = at<void *>(car, 1768);
    float point[3];
    matrices::transform_positions(1, &at<float>(car, wheel_offset + 2224), &at<float>(body, 692),
                                  &at<float>(body, 680), point, 12);
    surface_height(ground, point);
    point[2] = -point[2];
    if (!at<Dword>(ground, 24)) {
        particles::lifecycle::release_wheel_skid(car, index);
        return;
    }
    float width_factor = 1;
    if (style == 2) {
        width_factor = 2.25f;
        strength = 1;
    } else if (style == 3) {
        strength = 1;
    } else {
        if (style == 4)
            width_factor = 1.5f;
        if (strength < 0)
            strength = 0;
        else if (strength > 1)
            strength = 1;
    }
    void *owner = system(116);
    if (!at<void *>(car, wheel_offset + 2240))
        at<void *>(car, wheel_offset + 2240) = skids::lifecycle::allocate_skid_path(owner);
    if (void *path = at<void *>(car, wheel_offset + 2240)) {
        const float angle = float(E(at<float>(car, wheel_offset + 1968)) * 6.283185482025146484375f);
        float sine = float(std::sin(double(angle)));
        if (sine < 0)
            sine = -sine;
        const auto tire_width =
            Extended::integer(at<int>(car, index < 2 ? 912 : 1188), format) * .001f;
        const float width = float((E(sine) * .303f + 1.f) * tire_width * width_factor);
        body = at<void *>(car, 1768);
        skids::lifecycle::update_skid(owner, path, point, at<float>(body, 8), at<float>(body, 12),
                                      at<float>(body, 16), width, Dword(style), strength);
    }
    std::memcpy(bytes(car) + 1720 + Dword(index) * 12, point, 12);
    at<Byte>(car, 1695 + Dword(index)) = Byte(style);
}
} // namespace nfs::game::runtime::physics::contacts_pending
