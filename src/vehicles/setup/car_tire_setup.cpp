#include "car_initialization_runtime.hpp"
namespace nfs::game::runtime::ai::entities::pending {
kernel::numeric::Extended direction_turns(float, float);
}
namespace nfs::game::world::vehicle_setup {

void initialize_tire_geometry(void *car) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    auto I = [&](int value) { return Extended::integer(value, format); };
    auto radius = [&](unsigned parameters) {
        const auto sidewall =
            I(at<int>(car, parameters + 4)) * .01f * (I(at<int>(car, parameters)) * .001f);
        return float((sidewall + sidewall + I(at<int>(car, parameters + 8)) * .0254f) * .5f);
    };
    const float front_radius = radius(912);
    at<float>(car, 352) = at<float>(car, 2164) = at<float>(car, 2460) = front_radius;
    const float rear_radius = radius(1188);
    at<float>(car, 356) = at<float>(car, 3052) = at<float>(car, 2756) = rear_radius;
    at<Dword>(car, 2236) = at<Dword>(car, 2532) = at<Dword>(car, 1184);
    at<Dword>(car, 3124) = at<Dword>(car, 2828) = at<Dword>(car, 1460);
    for (unsigned wheel = 0; wheel < 4; ++wheel) {
        const unsigned anchor = 2160 + wheel * 296;
        auto W = [&](int index) -> Dword & { return at<Dword>(car, anchor + Dword(index * 4)); };
        auto F = [&](int index) -> float & { return at<float>(car, anchor + Dword(index * 4)); };
        at<Byte>(car, anchor + 44) = 1;
        W(-20) = 1;
        W(0) = at<Dword>(car, wheel < 2 ? 648 : 652);
        W(-49) = W(-48) = W(13) = 0;
        W(7) = W(8) = W(9) = 0;
        F(5) = float(E(at<float>(car, 520)) * .25f);
        F(-19) = F(-18) = 1;
        for (int index : {-14, -13, -12, -11, -10, -9})
            W(index) = 0;
        const float spin_inertia = float(E(F(1)) * F(1) * 15.f);
        F(-8) = spin_inertia;
        W(4) = W(6) = W(-17) = W(-16) = W(-15) = W(-42) = 0;
        const auto width = I(at<int>(car, wheel < 2 ? 912 : 1188)) * .001f;
        const auto transverse_inertia = width * width * 0.083333335816860198974609375f + spin_inertia;
        F(-7) = F(-6) = float(transverse_inertia);
        F(-5) = float(E(1) / E(spin_inertia));
        F(-4) = F(-3) = float(E(1) / transverse_inertia);
        const float width_product = float(width * 4.651162624359130859375f);
        float size = float(E(width_product).square_root());
        F(14) = size;
        if (size < .9f)
            size = .9f;
        else if (size > 1.5f)
            size = 1.5f;
        F(14) = size;
        for (int index : {-46, -43, -44, -45, -2, -1, 10, 12})
            W(index) = 0;
    }
    const Byte rear_left_driven = at<Byte>(car, 571);
    const Byte front_right_driven = at<Byte>(car, 569);
    at<Byte>(car, 3057) = rear_left_driven;
    const Byte rear_right_driven = at<Byte>(car, 570);
    at<float>(car, 1972) = at<float>(car, 2268) =
        float(E(at<float>(car, 928)) * 0.0055555556900799274444580078125f);
    at<Byte>(car, 2169) = at<Byte>(car, 568);
    at<Byte>(car, 2465) = front_right_driven;
    at<Byte>(car, 2761) = rear_right_driven;
    at<float>(car, 2860) = at<float>(car, 2564) =
        float(E(at<float>(car, 1204)) * 0.0055555556900799274444580078125f);
    if (at<Byte>(car, 568) && front_right_driven) {
        if (!(rear_left_driven && rear_right_driven)) {
            if (!rear_left_driven && !rear_right_driven)
                at<float>(car, 560) = 1;
            else
                at<Dword>(car, 560) = 0;
        }
    } else
        at<Dword>(car, 560) = 0;
    at<Dword>(car, 2236) = at<Dword>(car, 2532) = at<Dword>(car, 1184);
    at<Dword>(car, 3124) = at<Dword>(car, 2828) = at<Dword>(car, 1460);
    at<Byte>(car, 2464) = at<Byte>(car, 2168) = 1;
    at<Byte>(car, 3056) = at<Byte>(car, 2760) = at<Byte>(car, 584) ? 1 : 0;
    at<Dword>(car, 2220) = at<Dword>(car, 2516) = at<Dword>(car, 936);
    at<Dword>(car, 2172) = at<Dword>(car, 2468) = at<Dword>(car, 924);
    at<Dword>(car, 3108) = at<Dword>(car, 2812) = at<Dword>(car, 1212);
    at<Dword>(car, 3060) = at<Dword>(car, 2764) = at<Dword>(car, 1200);
    const float body_height = at<float>(at<void *>(car, 1768), 124);
    const auto front_half_width = E(at<float>(car, 532)) * .5f;
    const float rear_half_width = float(E(at<float>(car, 536)) * .5f);
    auto front_difference = E(at<float>(car, 352)) - at<float>(car, 356);
    if (E(0) < front_difference)
        front_difference = E(0);
    const float front_offset = float(front_difference);
    auto rear_difference = E(at<float>(car, 356)) - at<float>(car, 352);
    if (E(0) < rear_difference)
        rear_difference = E(0);
    const float rear_offset = float(rear_difference);
    at<float>(car, 2224) = float(-front_half_width);
    at<Dword>(car, 2824) = at<Dword>(car, 3120) = at<Dword>(car, 108);
    at<float>(car, 2816) = rear_half_width;
    const float front_y = float(-(E(front_offset) + body_height));
    at<float>(car, 2228) = at<float>(car, 2524) = front_y;
    at<Dword>(car, 2232) = at<Dword>(car, 2528) = at<Dword>(car, 104);
    at<float>(car, 2520) = float(front_half_width);
    const float rear_y = float(-(E(body_height) + rear_offset));
    at<float>(car, 2820) = at<float>(car, 3116) = rear_y;
    at<float>(car, 3112) = -rear_half_width;
    adjust_body_for_tires(car);
    const float front_angle =
        float(runtime::ai::entities::pending::direction_turns(front_offset, at<float>(car, 528)));
    at<float>(car, 360) = float(E(front_angle) - runtime::ai::entities::pending::direction_turns(
                                                       rear_offset, at<float>(car, 528)));
}
} // namespace nfs::game::world::vehicle_setup
