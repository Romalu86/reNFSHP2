#include "race_frame_draw.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::world::frame {
using kernel::numeric::Extended;
float measured_frame_rate;
float drawing_frame_rate;

float sample_frame_rate() {
    return measured_frame_rate;
}

void wrap_wheel_turn(float *value) {
    Dword bits;
    std::memcpy(&bits, value, 4);
    if ((bits | 2147483648u) == 4290772992u) {
        *value = 0;
        return;
    }
    const auto format = kernel::numeric::current_format();
    auto E = [&](float number) { return Extended(number, format); };
    // FRNDINT here starts with a float. Outside this interval it is already integral.
    auto rounded = [&](float number) {
        const auto input = E(number);
        return -8388608.0f < number && number < 8388608.0f
                   ? Extended::integer(input.round_to_integer(), format) : input;
    };
    const float input = *value;
    if (input > 1) {
        auto integral = rounded(input);
        if (E(input) < integral)
            integral = integral - 1.f;
        const float whole = float(integral);
        *value = float(E(*value) - whole);
    } else if (input < 0) {
        const float magnitude = -input;
        auto integral = rounded(magnitude);
        if (integral < E(magnitude))
            integral = integral + 1.f;
        const float whole = float(integral);
        *value = float(E(whole) + *value);
    }
}

void advance_wheel_turns(void *car, float maximum_rate) {
    const Dword ticks = graphics::unresolved::camera_state - graphics::previous_frame_tick;
    if (!ticks)
        return;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float number) { return Extended(number, format); };
    const float step = float(Extended::integer(std::int32_t(ticks), format) * 0.01666666753590106964111328125f);
    const float minimum_rate = -maximum_rate;
    auto advance = [&](unsigned velocity_offset, unsigned turn_offset) {
        auto rate = E(at<float>(car, velocity_offset)) * 0.15915493667125701904296875f;
        if (rate < E(minimum_rate))
            rate = E(minimum_rate);
        else if (E(maximum_rate) < rate)
            rate = E(maximum_rate);
        if (double(rate) == 0)
            return;
        const float delta = float(-(rate * step));
        const float previous = at<float>(car, turn_offset);
        auto maximum = E(0.5f);
        if (drawing_frame_rate > 0)
            maximum = maximum * drawing_frame_rate * 0.01666666753590106964111328125f;
        const float rounded_maximum = float(maximum);
        auto increment = E(delta);
        if (increment < -maximum)
            increment = -maximum;
        else if (E(rounded_maximum) < increment)
            increment = E(rounded_maximum);
        at<float>(car, turn_offset) = float(increment + previous);
        wrap_wheel_turn(&at<float>(car, turn_offset));
    };
    advance(2104, 1624);
    advance(2992, 1620);
}

void advance_renderable_wheels(void *renderable) {
    void *car = at<void *>(renderable, 3976);
    if (car && at<float>(renderable, 4048) != 0)
        advance_wheel_turns(car, at<float>(renderable, 4048));
}

void advance_vehicle_renderables(void *manager) {
    int remaining = at<int>(manager, 81304);
    if (remaining <= 0)
        return;
    Byte *renderable = at<Byte *>(manager, 81268);
    do {
        advance_renderable_wheels(renderable);
        renderable += 4064;
    } while (--remaining);
}
} // namespace nfs::game::world::frame
