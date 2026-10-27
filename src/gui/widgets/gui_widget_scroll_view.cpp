#include "gui_widget_text.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;
using kernel::numeric::Extended;

int clamp_gui_integer(int value, int lower, int upper) {
    if (upper < lower)
        lower = upper;
    if (value < lower)
        return lower;
    return value <= upper ? value : upper;
}

long double clamp_gui_float(float value, float lower, float upper) {
    if (upper < lower)
        lower = upper;
    if (value < lower)
        return lower;
    return upper < value ? upper : value;
}
namespace {
int scroll_thumb_position(Widget *bar) {
    const auto format = kernel::numeric::current_format();
    const Extended track = Extended::integer(at<int>(bar, 304), format);
    const Extended fraction(at<float>(bar, 276), format);
    const Extended position = Extended::integer(at<int>(bar, 280), format);
    const Extended range =
        Extended::integer(int(at<Dword>(bar, 268) - at<Dword>(bar, 272)), format);
    const float pixels = float((track - track * fraction) * (position / range));
    return int(std::lrintf(pixels));
}
} // namespace

void set_scroll_bar_position(Widget *bar, int position) {
    at<int>(bar, 280) = clamp_gui_integer(position, at<int>(bar, 272), at<int>(bar, 268));
    Widget *thumb = find_child(bar, at<Dword>(bar, 316));
    if (!thumb || application::pending::captured_widget_id != thumb->id)
        at<int>(bar, 284) = scroll_thumb_position(bar);
}

void __fastcall scroll_view_prepare_draw(Widget *widget, void *, Dword render_address) {
    prepare_draw(widget, nullptr, reinterpret_cast<engine::Engine *>(render_address));
    Rectangle extent;
    engine::construct_rectangle(&extent);
    int count = 0;
    for (int index = 0; index < int(widget->children.count); ++index)
        if (Widget *child = child_at(widget, index, 0)) {
            Rectangle bounds;
            engine::construct_rectangle(&bounds);
            if (!(child->flags & 40) || (child->layout_flags & 128)) {
                Rectangle translated;
                engine::translate_rectangle(&translated, &child->bounds, &widget->scroll);
                bounds.top = translated.top;
                bounds.left = translated.left;
                bounds.width = translated.width;
                bounds.height = translated.height;
                engine::destroy_rectangle(&translated);
            }
            if (!count) {
                extent.top = bounds.top;
                extent.left = bounds.left;
                extent.width = bounds.width;
                extent.height = bounds.height;
            } else
                union_rectangle(&extent, &bounds);
            ++count;
            engine::destroy_rectangle(&bounds);
        }
    if (!count) {
        Rectangle bounds;
        client_bounds(widget, &bounds);
        extent.top = bounds.top;
        extent.left = bounds.left;
        extent.width = bounds.width;
        extent.height = bounds.height;
        engine::destroy_rectangle(&bounds);
    }
    at<int>(widget, 268) = extent.top;
    at<int>(widget, 272) = extent.left;
    at<int>(widget, 280) = extent.height;
    at<int>(widget, 276) = extent.width;
    engine::destroy_rectangle(&extent);
}

void __fastcall scroll_view_scrollbar(Widget *widget, void *, Dword bar_address) {
    auto *bar = reinterpret_cast<Widget *>(bar_address);
    Rectangle viewport;
    client_bounds(widget, &viewport);
    Point scroll = widget->scroll;
    scroll.methods = engine::point_methods;
    const bool horizontal = (at<Dword>(bar, 264) & 2) != 0;
    if (at<Dword>(widget, 296) & (horizontal ? 2 : 1)) {
        set_enabled(bar, 0);
        application::pending::set_widget_visible(bar, 0);
    } else {
        int step = at<int>(widget, horizontal ? 288 : 292);
        if (step < 2)
            step = 1;
        const int offset = (horizontal ? scroll.x : scroll.y) / step;
        const int visible = (horizontal ? viewport.width : viewport.height) / step;
        const int total = at<int>(widget, horizontal ? 276 : 280) / step;
        const int maximum = int(Dword(total) - Dword(visible));
        if (maximum <= 0) {
            set_enabled(bar, 0);
            application::pending::set_widget_visible(bar, 0);
        } else {
            set_enabled(bar, 1);
            application::pending::set_widget_visible(bar, 1);
            at<Dword>(bar, 272) = 0;
            at<int>(bar, 268) = maximum;
            const auto format = kernel::numeric::current_format();
            const float visible_fraction =
                float(Extended::integer(visible, format) / Extended::integer(total, format));
            const Extended minimum_fraction = Extended::integer(at<int>(bar, 332), format) /
                                              Extended::integer(at<int>(bar, 304), format);
            const float fraction = Extended(visible_fraction, format) < minimum_fraction
                                       ? float(minimum_fraction)
                                       : visible_fraction;
            at<float>(bar, 276) = float(clamp_gui_float(fraction, 0, 1));
            set_scroll_bar_position(bar, at<int>(bar, 280));
            at<int>(bar, 280) = clamp_gui_integer(int(Dword(0) - Dword(offset)),
                                                    at<int>(bar, 272), at<int>(bar, 268));
            Widget *thumb = find_child(bar, at<Dword>(bar, 316));
            if (!thumb || application::pending::captured_widget_id != thumb->id)
                at<int>(bar, 284) = scroll_thumb_position(bar);
            const int beginning = at<int>(widget, horizontal ? 272 : 268);
            const int length = at<int>(widget, horizontal ? 276 : 280);
            const int end = int(Dword(horizontal ? viewport.left : viewport.top) +
                                Dword(horizontal ? viewport.width : viewport.height));
            if (Widget *plus = find_child(bar, at<Dword>(bar, 320)))
                set_enabled(plus, end < int(Dword(length) + Dword(beginning)));
            const Dword minus_enabled = at<int>(widget, horizontal ? 272 : 268) <
                                       (horizontal ? viewport.left : viewport.top);
            if (Widget *minus = find_child(bar, at<Dword>(bar, 324)))
                set_enabled(minus, minus_enabled);
        }
    }
    engine::destroy_point(&scroll);
    engine::destroy_rectangle(&viewport);
}
namespace {
Dword scroll_view_axis(Widget *widget, Dword command, int position, bool horizontal) {
    if (at<Dword>(widget, 296) & (horizontal ? 2 : 1))
        return 0;
    const int preserved = horizontal ? widget->scroll.y : widget->scroll.x;
    Dword offset = Dword(horizontal ? widget->scroll.x : widget->scroll.y);
    Rectangle viewport;
    client_bounds(widget, &viewport);
    const Dword length = Dword(horizontal ? viewport.width : viewport.height);
    engine::destroy_rectangle(&viewport);
    const Dword step = at<Dword>(widget, horizontal ? 288 : 292);
    Dword page = length - step;
    if (int(step) >= int(page))
        page = step;
    const Dword extent = at<Dword>(widget, horizontal ? 276 : 280);
    switch (command) {
    case 0:
        offset += step;
        break;
    case 1:
        offset -= step;
        break;
    case 2:
        offset += page;
        break;
    case 3:
        offset -= page;
        break;
    case 4:
        offset = Dword(0) - step * Dword(position);
        break;
    case 5:
        offset = 0;
        break;
    case 6:
        offset = length - extent;
        break;
    }
    Dword lower = length - extent;
    if (int(lower) > 0)
        lower = 0;
    if (int(lower) <= int(offset))
        lower = int(offset) > 0 ? 0 : offset;
    if (horizontal) {
        widget->scroll.y = preserved;
        widget->scroll.x = int(lower);
    } else {
        widget->scroll.x = preserved;
        widget->scroll.y = int(lower);
    }
    return 1;
}
} // namespace

Dword __fastcall scroll_view_scroll_command(Widget *widget, void *, Dword command, Dword position) {
    return scroll_view_axis(widget, command, int(position), false);
}

Dword __fastcall scroll_view_scroll_notify(Widget *widget, void *, Dword command, Dword position) {
    return scroll_view_axis(widget, command, int(position), true);
}

Dword __fastcall scroll_view_key_down(Widget *widget, void *, Dword key, Dword modifiers) {
    if (!(modifiers & 3145728)) {
        Dword command;
        switch (key) {
        case 199:
            command = 5;
            break;
        case 200:
            command = 0;
            break;
        case 201:
            command = 2;
            break;
        case 207:
            command = 6;
            break;
        case 208:
            command = 1;
            break;
        case 209:
            command = 3;
            break;
        default:
            return pending::general_key(reinterpret_cast<GeneralWidget *>(widget), nullptr, key,
                                        modifiers);
        }
        events::post_widget(widget, 39, command, 0);
        return 1;
    }
    return pending::general_key(reinterpret_cast<GeneralWidget *>(widget), nullptr, key, modifiers);
}
} // namespace nfs::gui::widgets::family
