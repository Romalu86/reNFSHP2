#include "gui_widget_interaction.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
#include <cstdlib>
namespace nfs::gui::widgets::pending {
const char *__cdecl scroll_bar_class_name();
}
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;
using kernel::numeric::Extended;
namespace {
Dword scroll_drag_initialized{};
int scroll_drag_offset{};
Point scroll_drag_origin{};
// Destructor tail restores the base table.
void destroy_scroll_drag_origin() {
    engine::destroy_point(&scroll_drag_origin);
}
Rectangle child_rectangle(Widget *widget) {
    return widget ? Rectangle{engine::rectangle_methods, widget->bounds.top, widget->bounds.left,
                              widget->bounds.width, widget->bounds.height}
                  : Rectangle{engine::rectangle_methods, 0, 0, 0, 0};
}
int clamp_thumb_pixel(int pixel, int lower, int upper) {
    const auto format = kernel::numeric::current_format();
    const Extended end(float(Extended::integer(upper, format)), format);
    Extended start = Extended::integer(lower, format);
    const Extended value(float(Extended::integer(pixel, format)), format);
    if (end < start)
        start = end;
    const Extended result = value < start ? start : end < value ? end : value;
    return int(result.truncate_to_integer());
}
} // namespace

void __fastcall scroll_bar_initialize(Widget *widget, void *) {
    auto *bar = reinterpret_cast<ScrollBar *>(widget);
    if (widget->parent)
        events::post_widget(widget->parent, 37, widget->id, 0);
    if (!(bar->style & 1)) {
        bar->corner = named_child_id(widget, "<CORNER>");
        bar->minus = named_child_id(widget, "<MINUS>");
        bar->page_up = named_child_id(widget, "<PGUP>");
        bar->thumb = named_child_id(widget, "<TAB>");
        bar->page_down = named_child_id(widget, "<PGDN>");
        bar->plus = named_child_id(widget, "<PLUS>");
    } else {
        set_widget_resource(widget, &lifecycle::construction::unresolved::shared_string);
        const int size = scale_gui_distance(16);
        auto *corner = static_cast<Widget *>(create_image_box(widget));
        const Rectangle square{engine::rectangle_methods, 0, 0, size, size};
        engine::copy_rectangle_fields(&corner->bounds, &square);
        corner->flags = 4193;
        corner->colour = 4288651167u;
        set_z_order(corner, 5);
        at<Dword>(corner, 268) = 1;
        initialize_tree(corner);
        bar->corner = corner->id;
        auto make_button = [&](int order, bool initialize) {
            auto *button = static_cast<Widget *>(create_button(widget));
            engine::copy_rectangle_fields(&button->bounds, &square);
            button->colour = 4288651167u;
            button->flags = 4704;
            set_z_order(button, Dword(order));
            reinterpret_cast<Button *>(button)->style = 9;
            if (initialize)
                initialize_tree(button);
            return button;
        };
        bar->page_down = make_button(4, true)->id;
        bar->page_up = make_button(3, true)->id;
        bar->plus = make_button(2, true)->id;
        bar->minus = make_button(1, false)->id;
        auto *thumb = static_cast<Widget *>(create_image_box(widget));
        engine::copy_rectangle_fields(&thumb->bounds, &square);
        thumb->flags = 4704;
        thumb->colour = 4288651167u;
        set_z_order(thumb, 0);
        at<Dword>(thumb, 268) = 8;
        initialize_tree(thumb);
        bar->thumb = thumb->id;
    }
    if (bar->style & 4)
        widget->flags |= 32;
    layout_scroll_bar(widget);
    pending::request_general_layout(reinterpret_cast<GeneralWidget *>(widget), nullptr);
}

void __fastcall scroll_bar_configure_geometry(Widget *widget, void *, Dword sender_address,
                                              Dword event, Dword state, Dword modifiers) {
    auto *bar = reinterpret_cast<ScrollBar *>(widget);
    Widget *parent = widget->parent;
    const Dword sender = reinterpret_cast<Widget *>(sender_address)->id;
    if (!(scroll_drag_initialized & 1)) {
        scroll_drag_initialized |= 1;
        engine::construct_point(&scroll_drag_origin);
        std::atexit(destroy_scroll_drag_origin);
    }
    const Dword notification = ((~bar->style & 2) | 76) >> 1;
    Widget *thumb = find_child(widget, bar->thumb);
    const int thumb_size =
        thumb ? (bar->style & 2 ? thumb->bounds.width : thumb->bounds.height) : 0;
    if (!parent)
        return;
    if (sender == bar->plus && event == 43)
        events::post_widget(parent, notification, 1, Dword(bar->position));
    if (sender == bar->minus && event == 43)
        events::post_widget(parent, notification, 0, 0);
    if (sender == bar->page_up && event == 43)
        events::post_widget(parent, notification, 2, Dword(bar->position));
    if (sender == bar->page_down && event == 43)
        events::post_widget(parent, notification, 3, Dword(bar->position));
    if (sender == bar->thumb && event == 11 && !state && !std::uint8_t(modifiers)) {
        Point screen = *pending::pointer_position();
        screen.methods = engine::point_methods;
        Point local;
        from_screen_point(widget, &local, &screen);
        scroll_drag_origin.x = local.x;
        scroll_drag_origin.y = local.y;
        scroll_drag_offset = bar->thumb_offset;
        capture_widget(reinterpret_cast<Widget *>(sender_address));
        engine::destroy_point(&local);
    }
    const Dword thumb_id = bar->thumb;
    if (sender != thumb_id)
        return;
    if (event == 12 && !state)
        release_capture(reinterpret_cast<Widget *>(sender_address));
    if (sender != thumb_id || event != 10 ||
        application::pending::captured_widget_id !=
            reinterpret_cast<Widget *>(sender_address)->id ||
        !(modifiers & 4194304))
        return;
    const bool horizontal = (bar->style & 2) != 0;
    int length = horizontal ? bar->track.width : bar->track.height;
    if (!length || bar->maximum <= bar->minimum)
        return;
    Point screen = *pending::pointer_position();
    screen.methods = engine::point_methods;
    Point local;
    from_screen_point(widget, &local, &screen);
    const int delta = int(Dword(horizontal ? local.x : local.y) -
                          Dword(horizontal ? bar->track.left : scroll_drag_origin.y));
    const int pixel = int(Dword(scroll_drag_offset) + Dword(delta));
    bar->thumb_offset = pixel;
    length = horizontal ? bar->track.width : bar->track.height;
    bar->thumb_offset = clamp_gui_integer(pixel, 0, length);
    const auto format = kernel::numeric::current_format();
    Extended fraction = Extended::integer(bar->thumb_offset, format) /
                        Extended::integer(int(Dword(length) - Dword(thumb_size)), format);
    const Extended zero(0.0f, format), one(1.0f, format);
    if (fraction < zero)
        fraction = zero;
    else if (one < fraction)
        fraction = one;
    const int span = int(Dword(bar->maximum) - Dword(bar->minimum) + (horizontal ? 0 : 1));
    const int value =
        clamp_gui_integer(int(std::lrintf(float(Extended::integer(span, format) * fraction))),
                          bar->minimum, bar->maximum);
    engine::destroy_point(&local);
    if (value == bar->position)
        return;
    bar->position = value;
    if (value == bar->minimum)
        events::dispatch(parent, notification, 5, 0);
    else if (value == bar->maximum)
        events::dispatch(parent, notification, 6, 0);
    else
        events::dispatch(parent, notification, 4, Dword(value));
}

void layout_scroll_bar(Widget *widget) {
    auto *bar = reinterpret_cast<ScrollBar *>(widget);
    if (!widget->parent)
        return;
    Widget *peer = nullptr;
    if (!(bar->style & 4))
        for (int index = 0; index < int(widget->parent->children.count); ++index) {
            Widget *child = child_at(widget->parent, index, 0);
            if (child && child != widget && !(child->flags & 2) &&
                invoke<Dword>(child, 2,
                             reinterpret_cast<const void *>(pending::scroll_bar_class_name)) &&
                ((at<Dword>(child, 264) ^ bar->style) & 2) && !(at<Dword>(child, 264) & 4)) {
                peer = child;
                break;
            }
        }
    Rectangle client;
    client_bounds(widget->parent, &client);
    Rectangle bounds = widget->bounds;
    Widget *minus = find_child(widget, bar->minus), *page_up = find_child(widget, bar->page_up),
           *thumb = find_child(widget, bar->thumb), *page_down = find_child(widget, bar->page_down),
           *plus = find_child(widget, bar->plus), *corner = find_child(widget, bar->corner);
    Rectangle minus_bounds = child_rectangle(minus), up_bounds = child_rectangle(page_up),
              thumb_bounds = child_rectangle(thumb), down_bounds = child_rectangle(page_down),
              plus_bounds = child_rectangle(plus);
    const Dword style = bar->style;
    const bool horizontal = (style & 2) != 0;
    if (!(style & 8)) {
        Rectangle united = minus_bounds;
        union_rectangle(&united, &up_bounds);
        union_rectangle(&united, &thumb_bounds);
        union_rectangle(&united, &down_bounds);
        union_rectangle(&united, &plus_bounds);
        if (!horizontal) {
            bounds.top = client.top;
            bounds.width = united.width;
            bounds.left = style & 4 ? client.width : int(Dword(client.width) - Dword(bounds.width));
            bounds.height = client.height;
            if (peer && !(style & 4))
                bounds.height = int(Dword(client.height) - Dword(peer->bounds.height));
        } else {
            bounds.left = client.left;
            bounds.height = united.height;
            bounds.top = style & 4
                             ? int(Dword(client.height) + Dword(client.top))
                             : int(Dword(client.height) - Dword(bounds.height) + Dword(client.top));
            bounds.width = client.width;
            if (peer && !(style & 4))
                bounds.width = int(Dword(client.width) - Dword(peer->bounds.width));
        }
        engine::destroy_rectangle(&united);
    }
    const auto format = kernel::numeric::current_format();
    if (!horizontal) {
        minus_bounds.top = bounds.top;
        plus_bounds.top = int(Dword(bounds.height) - Dword(plus_bounds.height));
        bar->track.top = minus_bounds.height;
        bar->track.left = 0;
        bar->track.width = bounds.width;
        const int length =
            int(Dword(bounds.height) - Dword(minus_bounds.height) - Dword(plus_bounds.height));
        bar->track.height = length;
        int size = length;
        if (bar->thumb_fraction != 0)
            size = int(std::lrintf(
                float(Extended::integer(length, format) * Extended(bar->thumb_fraction, format))));
        const int start = bar->track.top;
        const int pixel =
            clamp_thumb_pixel(int(Dword(bar->thumb_offset) + Dword(start)), start,
                              int(Dword(start) - Dword(size) - 1 + Dword(bar->track.height)));
        thumb_bounds.top = pixel;
        thumb_bounds.height = size;
        up_bounds.top = start;
        up_bounds.height = int(Dword(pixel) - Dword(start));
        down_bounds.top = int(Dword(pixel) + Dword(size));
        down_bounds.height =
            int(Dword(bar->track.height) + Dword(bar->track.top) - Dword(pixel) - Dword(size));
    } else {
        minus_bounds.left = bounds.left;
        plus_bounds.left = int(Dword(bounds.width) - Dword(plus_bounds.width));
        bar->track.left = minus_bounds.width;
        bar->track.height = bounds.height;
        bar->track.top = 0;
        const int length =
            int(Dword(bounds.width) - Dword(minus_bounds.width) - Dword(plus_bounds.width));
        bar->track.width = length;
        int size = length;
        if (bar->thumb_fraction != 0)
            size = int(std::lrintf(
                float(Extended::integer(length, format) * Extended(bar->thumb_fraction, format))));
        const int start = bar->track.left;
        const int pixel =
            clamp_thumb_pixel(int(Dword(bar->thumb_offset) + Dword(start)), start,
                              int(Dword(bar->track.width) - Dword(size) - 1 + Dword(start)));
        thumb_bounds.left = pixel;
        thumb_bounds.width = size;
        up_bounds.left = start;
        up_bounds.width = int(Dword(pixel) - Dword(start));
        down_bounds.left = int(Dword(pixel) + Dword(size));
        down_bounds.width =
            int(Dword(bar->track.width) + Dword(bar->track.left) - Dword(pixel) - Dword(size));
    }
    if (corner) {
        application::pending::set_widget_visible(corner, peer && !horizontal);
        if (peer && !horizontal) {
            corner->bounds.left = 0;
            corner->bounds.top = int(Dword(bounds.top) + Dword(bounds.height));
        }
    }
    if (minus)
        engine::copy_rectangle_fields(&minus->bounds, &minus_bounds);
    if (page_up)
        engine::copy_rectangle_fields(&page_up->bounds, &up_bounds);
    if (thumb)
        engine::copy_rectangle_fields(&thumb->bounds, &thumb_bounds);
    if (page_down)
        engine::copy_rectangle_fields(&page_down->bounds, &down_bounds);
    if (plus)
        engine::copy_rectangle_fields(&plus->bounds, &plus_bounds);
    engine::copy_rectangle_fields(&widget->bounds, &bounds);
    engine::destroy_rectangle(&client);
}

void __fastcall scroll_bar_prepare_draw(Widget *widget, void *, Dword render_address) {
    if (widget->parent && visible_in_ancestors(widget)) {
        events::dispatch(widget->parent, 37, widget->id, 0);
        if (!(widget->flags & 2))
            layout_scroll_bar(widget);
    }
    prepare_draw(widget, nullptr, reinterpret_cast<engine::Engine *>(render_address));
}
} // namespace nfs::gui::widgets::family
