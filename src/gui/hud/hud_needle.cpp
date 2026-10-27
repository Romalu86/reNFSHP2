#include "hud_instruments.hpp"
#include "../../ai/driving/ai_route_planner.hpp"
#include <cmath>
namespace nfs::application::frontend::hud {
using namespace gui::widgets;
using kernel::numeric::Extended;
const gui::engine::pending::FlagDefinition needle_styles[] = {
    {"MNS_FLAT", 0, 1}, {"MNS_TOP", 0, 2}, {"MNS_SUBCLIP", 0, 4},
    {"MNS_360", 0, 8}, {"MNS_FLIP", 0, 16}, {nullptr, 0, 0}};

Dword __fastcall load_needle(Needle *needle, void *, void *configuration) {
    using namespace gui::engine::pending;
    read_scene_vector(configuration, "MNeedle.mStart", &needle->start, &scene_default_vector);
    read_scene_vector(configuration, "MNeedle.mEnd", &needle->end, &scene_default_vector);
    read_image_resource(configuration, "MNeedle.mImage", &needle->image);
    read_flags(configuration, "MNeedle.mNeedleStyle", &needle->style, needle_styles, 0);
    read_rectangle(configuration, "MNeedle.mSubClip", &needle->subclip, &gui::engine::default_bounds);
    read_float(configuration, "MNeedle.mSpeed", &needle->speed, 5.f);
    needle->start_angle = float(game::runtime::ai::entities::pending::direction_turns(-needle->start.x, needle->start.z));
    needle->end_angle = float(game::runtime::ai::entities::pending::direction_turns(-needle->end.x, needle->end.z));
    if (needle->style & 8) {
        needle->start_angle = 0;
        needle->end_angle = 1;
    }
    if (needle->style & 16) {
        const auto format = kernel::numeric::current_format();
        if (needle->end_angle < needle->start_angle)
            needle->end_angle = float(Extended(needle->end_angle, format) + 1.f);
        else needle->start_angle = float(Extended(needle->start_angle, format) + 1.f);
    }
    return gui::widgets::pending::load_general(&needle->base, nullptr, configuration);
}

Dword __fastcall save_needle(Needle *needle, void *, void *configuration) {
    using namespace gui::engine::pending;
    write_scene_vector(configuration, "MNeedle.mStart", &needle->start, 2, &scene_default_vector);
    write_scene_vector(configuration, "MNeedle.mEnd", &needle->end, 2, &scene_default_vector);
    write_image_resource(configuration, "MNeedle.mImage", &needle->image);
    write_flags(configuration, "MNeedle.mNeedleStyle", needle->style, needle_styles, 0);
    write_rectangle(configuration, "MNeedle.mSubClip", &needle->subclip, &gui::engine::default_bounds);
    write_float(configuration, "MNeedle.mSpeed", needle->speed, 2, 5.f);
    return gui::widgets::pending::save_general(&needle->base, nullptr, configuration);
}

void rotate_needle_corners(gui::engine::Point *first, gui::engine::Point *second,
                           gui::engine::Point *third, gui::engine::Point *fourth,
                           float angle, int width, int height) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float f) { return Extended(f, format); };
    const float half_width = float(Extended::integer(width, format) * .5f);
    const float half_height = float(Extended::integer(height, format) * .5f);
    const auto cosine = Extended::from_double(double(game::runtime::turn_cosine(angle)), format);
    const auto sine = Extended::from_double(double(game::runtime::turn_sine(angle)), format);
    const float width_cosine = float(cosine) * half_width;
    const float height_cosine = float(cosine) * half_height;
    const float width_sine = float(E(half_width) * sine);
    const float height_sine = float(sine * half_height);
    const auto left_retained = E(half_width) - width_cosine;
    const float left = float(left_retained);
    const auto top_retained = E(half_height) - height_cosine;
    const float top = float(top_retained);
    const auto rounded = [&](const Extended &value) {
        const float stored = float(value);
        const auto integer = E(stored).round_to_integer();
        return integer < INT32_MIN || integer > INT32_MAX ? INT32_MIN : int(integer);
    };
    first->x = rounded(left_retained - height_sine);
    first->y = rounded(top_retained + width_sine);
    second->x = rounded((E(width_cosine) + half_width) - height_sine);
    second->y = rounded(E(top) - width_sine);
    third->x = rounded(E(height_sine) + width_cosine + half_width);
    third->y = rounded((E(height_cosine) + half_height) - width_sine);
    fourth->x = rounded(E(left) + height_sine);
    fourth->y = rounded(E(width_sine) + height_cosine + half_height);
}

void __fastcall tick_needle(Needle *needle, void *, Dword ticks) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float f) { return Extended(f, format); };
    auto difference = E(needle->target) - needle->value;
    const float stored_difference = float(difference);
    if (difference < E(0)) difference = -difference;
    const float magnitude = float(difference);
    // TEST AH,44 / JNP admits unordered values, but returns for ordered zero.
    if (double(difference) == 0) return;
    auto step = gui::ticks_to_seconds(int(ticks)) * needle->speed * magnitude;
    float distance = stored_difference;
    if (distance < 0) distance = -distance;
    if (needle->style & 8) {
        const bool crosses_zero = (needle->value > .75f && needle->target < .25f) ||
                                  (needle->target > .75f && needle->value < .25f);
        if (crosses_zero) distance = float(E(1) - distance);
        if (!(step < E(distance))) step = E(distance);
        const bool below = needle->target < needle->value;
        needle->value = float(crosses_zero ? (below ? E(needle->value) + step : E(needle->value) - step)
                                         : (below ? E(needle->value) - step : E(needle->value) + step));
        if (needle->value < 0) needle->value = float(E(needle->value) + 1.f);
        needle->value = float(std::fmod(double(needle->value), 1.0));
    } else {
        if (!(step < E(distance))) step = E(distance);
        if (needle->target > needle->value) needle->value = float(step + E(needle->value));
        else if (needle->target < needle->value) needle->value = float(E(needle->value) - step);
    }
    if (needle->value < 0) needle->value = 0;
    else if (needle->value > 1) needle->value = 1;
}

void set_needle_value(Needle *needle, float value, Dword interpolate) {
    if (value < 0) value = 0;
    else if (value > 1) value = 1;
    needle->target = value;
    if (!interpolate) needle->value = value;
}
}
