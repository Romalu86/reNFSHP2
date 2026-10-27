#include "gui_script.hpp"
#include "../widgets/gui_widget_interaction.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstdio>
#include <climits>
namespace nfs::gui::engine {
int seconds_to_ticks(float);
}
namespace nfs::gui::widgets::script::commands {
using engine::pending::string_integer;
using engine::pending::string_number;
using kernel::numeric::current_format;
using kernel::numeric::Extended;
namespace {
template <unsigned Count>
std::array<const String *, Count> values(Script *script, Command *command) {
    std::array<const String *, Count> result;
    for (unsigned index = 0; index < Count; ++index)
        result[index] = argument(command, index, script);
    return result;
}
template <unsigned Count> bool present(const std::array<const String *, Count> &arguments) {
    for (const String *value : arguments)
        if (!string_data(value))
            return false;
    return true;
}
int rounded_integer(float value) {
    const auto result = Extended(value, current_format()).round_to_integer();
    return result < INT_MIN || result > INT_MAX ? INT_MIN : int(result);
}
int increment(Command *command, int ticks, int duration) {
    const int remaining = int(Dword(duration) - Dword(command->elapsed));
    const int consumed = ticks < remaining ? ticks : remaining;
    return command->elapsed = int(Dword(command->elapsed) + Dword(consumed));
}
float fraction(Command *command, int duration) {
    const auto format = current_format();
    return float(Extended::integer(command->elapsed, format) / Extended::integer(duration, format));
}
int interpolate(int first, int last, float progress) {
    const auto format = current_format();
    const int delta = int(Dword(last) - Dword(first));
    const float scaled = float(Extended::integer(delta, format) * Extended(progress, format));
    return int(Dword(first) + Dword(rounded_integer(scaled)));
}
Dword transform_value(const Extended &value) {
    return Dword((value * 65536.0f).truncate_to_integer());
}
} // namespace

Dword __cdecl move(Script *script, Command *command, int ticks) {
    Widget *widget = target(script);
    const auto arguments = values<5>(script, command);
    if (widget && present(arguments)) {
        const int left = string_integer(arguments[0]), top = string_integer(arguments[1]);
        const int end_left = string_integer(arguments[2]), end_top = string_integer(arguments[3]);
        const float seconds = float(string_number(arguments[4]));
        const int duration = engine::seconds_to_ticks(seconds);
        if (seconds > 0.0f && duration > 0) {
            increment(command, ticks, duration);
            const float progress = fraction(command, duration);
            const int current_left = interpolate(left, end_left, progress);
            const int current_top = interpolate(top, end_top, progress);
            volatile Rectangle &bounds = widget->bounds;
            bounds.left = current_left;
            bounds.top = current_top;
            if (command->elapsed < duration)
                return 0;
            command->elapsed = 0;
        }
        widget->bounds.left = end_left;
        widget->bounds.top = end_top;
    }
    return 1;
}

Dword __cdecl grow(Script *script, Command *command, int ticks) {
    Widget *widget = target(script);
    const auto arguments = values<5>(script, command);
    if (widget && present(arguments)) {
        const int width = string_integer(arguments[0]), height = string_integer(arguments[1]);
        const int end_width = string_integer(arguments[2]),
                  end_height = string_integer(arguments[3]);
        const float seconds = float(string_number(arguments[4]));
        const int duration = engine::seconds_to_ticks(seconds);
        if (seconds > 0.0f && duration > 0) {
            increment(command, ticks, duration);
            const float progress = fraction(command, duration);
            const int current_width = interpolate(width, end_width, progress);
            const int current_height = interpolate(height, end_height, progress);
            widget->bounds.height = current_height;
            widget->bounds.width = current_width;
            if (command->elapsed < duration)
                return 0;
            command->elapsed = 0;
        } else {
            widget->bounds.width = end_width;
            widget->bounds.height = end_height;
        }
    }
    return 1;
}

Dword __cdecl move_grow(Script *script, Command *command, int ticks) {
    Widget *widget = target(script);
    const auto arguments = values<9>(script, command);
    if (widget && present(arguments)) {
        const int left = string_integer(arguments[0]), top = string_integer(arguments[1]);
        const int width = string_integer(arguments[2]), height = string_integer(arguments[3]);
        const int end_left = string_integer(arguments[4]), end_top = string_integer(arguments[5]);
        const int end_width = string_integer(arguments[6]),
                  end_height = string_integer(arguments[7]);
        const float seconds = float(string_number(arguments[8]));
        const int duration = engine::seconds_to_ticks(seconds);
        Rectangle bounds{engine::rectangle_methods, end_top, end_left, end_width, end_height};
        if (seconds > 0.0f && duration > 0) {
            increment(command, ticks, duration);
            const float progress = fraction(command, duration);
            bounds.left = interpolate(left, end_left, progress);
            bounds.top = interpolate(top, end_top, progress);
            bounds.width = interpolate(width, end_width, progress);
            bounds.height = interpolate(height, end_height, progress);
            engine::copy_rectangle_fields(&widget->bounds, &bounds);
            engine::destroy_rectangle(&bounds);
            if (command->elapsed < duration)
                return 0;
            command->elapsed = 0;
        } else {
            engine::copy_rectangle_fields(&widget->bounds, &bounds);
            engine::destroy_rectangle(&bounds);
        }
    }
    return 1;
}

Dword __cdecl fade(Script *script, Command *command, int ticks) {
    Widget *widget = target(script);
    const auto arguments = values<3>(script, command);
    if (widget && present(arguments)) {
        Dword first = widget->colour, last = widget->colour;
        std::sscanf(string_ascii(arguments[0]), "%x", &first);
        std::sscanf(string_ascii(arguments[1]), "%x", &last);
        const float seconds = float(string_number(arguments[2]));
        const int duration = engine::seconds_to_ticks(seconds);
        if (seconds > 0.0f && duration > 0 && first != last) {
            increment(command, ticks, duration);
            const float progress = fraction(command, duration);
            const auto format = current_format();
            const float inverse = float(Extended(1.0f, format) - Extended(progress, format));
            Dword colour = 0;
            for (unsigned shift : {24u, 16u, 8u, 0u}) {
                const auto from = Extended::integer((first >> shift) & 255, format);
                const auto to = Extended::integer((last >> shift) & 255, format);
                const float channel = float(from * inverse + to * progress);
                colour |= Dword(std::uint8_t(rounded_integer(channel))) << shift;
            }
            widget->colour = colour;
            if (command->elapsed < duration)
                return 0;
            command->elapsed = 0;
        }
        widget->colour = last;
    }
    return 1;
}

Dword __cdecl transform(Script *script, Command *command, int ticks) {
    Widget *widget = target(script);
    const auto arguments = values<3>(script, command);
    if (present(arguments) && widget) {
        const float first = float(string_number(arguments[0])),
                    last = float(string_number(arguments[1]));
        const float seconds = float(string_number(arguments[2]));
        const int duration = engine::seconds_to_ticks(seconds);
        const auto format = current_format();
        if (seconds > 0.0f && duration > 0 && first != last) {
            increment(command, ticks, duration);
            const auto progress =
                Extended::integer(command->elapsed, format) / Extended::integer(duration, format);
            const auto current = (Extended(1.0f, format) - progress) * first + progress * last;
            events::dispatch(widget, 51, transform_value(current), 2);
            if (command->elapsed < duration)
                return 0;
            events::dispatch(widget, 51, transform_value(Extended(last, format)), 2);
            command->elapsed = 0;
        } else {
            events::dispatch(widget, 51, transform_value(Extended(last, format)), 2);
        }
    }
    return 1;
}
} // namespace nfs::gui::widgets::script::commands
