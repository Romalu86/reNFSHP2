#include "gui_widget_list.hpp"
#include "nfs/core/application/application.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;
using kernel::numeric::Extended;

void __fastcall list_box_initialize(Widget *widget, void *) {
    ListBox *list = list_box(widget);
    list->columns = list->checks = list->unchecks = list->headers = 0;
    for (int index = 0; index < int(widget->children.count); ++index) {
        Widget *child = child_at(widget, index, 0);
        if (!child || !child->name.length)
            continue;
        if (std::strstr(string_ascii(&child->name), "<HEADER")) {
            int column;
            if (std::sscanf(string_ascii(&child->name), "<HEADER%d>", &column))
                list->headers |= Dword(1) << (Dword(column) & 31);
        }
        if (std::strstr(string_ascii(&child->name), "<COLUMN")) {
            int column;
            if (std::sscanf(string_ascii(&child->name), "<COLUMN%d>", &column))
                list->columns |= Dword(1) << (Dword(column) & 31);
            application::pending::set_widget_visible(child, 0);
        }
        if (std::strstr(string_ascii(&child->name), "<CHECK")) {
            int column;
            if (std::sscanf(string_ascii(&child->name), "<CHECK%d>", &column))
                list->checks |= Dword(1) << (Dword(column) & 31);
            application::pending::set_widget_visible(child, 0);
        }
        if (std::strstr(string_ascii(&child->name), "<UNCHECK")) {
            int column;
            if (std::sscanf(string_ascii(&child->name), "<UNCHECK%d>", &column))
                list->unchecks |= Dword(1) << (Dword(column) & 31);
            application::pending::set_widget_visible(child, 0);
        }
    }
    for (Dword column = 0; column < 32; ++column) {
        Widget *header = invoke<Widget *>(widget, 50, column);
        Widget *cell = invoke<Widget *>(widget, 47, column);
        if (cell && header) {
            header->bounds.left = cell->bounds.left;
            header->bounds.width = cell->bounds.width;
        }
    }
    pending::request_general_layout(reinterpret_cast<GeneralWidget *>(widget), nullptr);
}
// RET10h retains the fourth, unused argument.
void __fastcall list_box_configure_geometry(Widget *widget, void *, Dword source, Dword event,
                                            Dword state, Dword) {
    if (event == 43)
        for (Dword column = 0; column < 32; ++column)
            if (source == reinterpret_cast<Dword>(invoke<Widget *>(widget, 50, column))) {
                events::post_widget(widget, 67, column, state);
                return;
            }
}

void __fastcall list_box_prepare_draw(Widget *widget, void *, Dword render_address) {
    selected_list_row(&list_box(widget)->items);
    prepare_draw(widget, nullptr, reinterpret_cast<engine::Engine *>(render_address));
    auto *list = list_box(widget);
    if (list->fitted_rows && (list->style & 64)) {
        const Dword rows = list->items.grid.rows.count < list->fitted_rows
                              ? list->items.grid.rows.count
                              : list->fitted_rows;
        widget->bounds.height = int(Dword(list->row_height) * rows);
    }
}

void configure_scroll_bar_range(Widget *widget, int minimum, int maximum, float fraction) {
    auto *bar = reinterpret_cast<ScrollBar *>(widget);
    if (maximum <= minimum)
        maximum = minimum;
    bar->minimum = minimum;
    bar->maximum = maximum;
    const auto format = kernel::numeric::current_format();
    const float minimum_fraction = float(Extended::integer(bar->minimum_thumb_height, format) /
                                         Extended::integer(bar->track.height, format));
    if (fraction < minimum_fraction)
        fraction = minimum_fraction;
    bar->thumb_fraction = float(clamp_gui_float(fraction, 0, 1));
    set_scroll_bar_position(widget, bar->position);
}

Dword __fastcall list_box_scroll_command(Widget *widget, void *, Dword command, Dword position) {
    auto *list = list_box(widget);
    int row;
    switch (command) {
    case 0:
        row = first_visible_list_row(widget) - 1;
        break;
    case 1: {
        const int visible = visible_list_rows(widget);
        row = visible ? last_visible_list_row(widget, visible) + 1 : 0;
        break;
    }
    case 2: {
        const int visible = visible_list_rows(widget);
        row = first_visible_list_row(widget) - visible;
        break;
    }
    case 3: {
        const int visible = visible_list_rows(widget);
        if (!visible)
            row = visible_list_rows(widget) - 1;
        else {
            const int last = last_visible_list_row(widget, visible);
            row = last + visible_list_rows(widget);
        }
        break;
    }
    case 4: {
        const int x = widget->scroll.x;
        int distance = 0;
        if (!(list->style & 32))
            distance = int(Dword(list->row_height) * position);
        else
            for (int index = 0; index < int(position); ++index)
                distance = int(Dword(distance) + Dword(list_row_height(widget, index)));
        widget->scroll.x = x;
        widget->scroll.y = int(Dword(0) - Dword(distance));
        return 1;
    }
    case 5:
        row = 0;
        break;
    case 6:
        row = int(list->items.grid.rows.count - 1);
        break;
    default:
        return 1;
    }
    ensure_list_box_row_visible(widget, row);
    return 1;
}

void __fastcall list_box_scrollbar(Widget *widget, void *, Dword bar_address) {
    Widget *bar = reinterpret_cast<Widget *>(bar_address);
    if (!(at<Dword>(bar, 264) & 2)) {
        const int count = int(list_box(widget)->items.grid.rows.count);
        if (!count) {
            set_enabled(bar, 0);
            application::pending::set_widget_visible(bar, 0);
            notify_scroll(widget, nullptr, bar);
            return;
        }
        const int first = first_visible_list_row(widget);
        const int last = last_visible_list_row(widget, visible_list_rows(widget));
        const int visible = visible_list_rows(widget);
        if (count - visible < 1) {
            set_enabled(bar, 0);
            application::pending::set_widget_visible(bar, 0);
            notify_scroll(widget, nullptr, bar);
            return;
        }
        set_enabled(bar, 1);
        application::pending::set_widget_visible(bar, 1);
        const auto format = kernel::numeric::current_format();
        configure_scroll_bar_range(
            bar, 0, count - visible,
            float(Extended::integer(visible, format) / Extended::integer(count, format)));
        set_scroll_bar_position(bar, first);
        if (Widget *plus = find_child(bar, at<Dword>(bar, 320)))
            set_enabled(plus, last < count - 1);
        if (Widget *minus = find_child(bar, at<Dword>(bar, 324)))
            set_enabled(minus, 0 < first);
    }
    notify_scroll(widget, nullptr, bar);
}

Dword __fastcall list_box_event(Widget *widget, void *, Dword event, Dword first, Dword second) {
    switch (event) {
    case 42:
        if (second)
            widget->scroll.y = 0;
        return 1;
    case 65:
        return invoke<Dword>(widget, 55, first, second & 65535, second >> 16);
    case 66:
        invoke(widget, 56, first, second & 65535, second >> 16);
        return 1;
    case 68:
        invoke(widget, 57);
        return 1;
    default:
        return pending::general_event(reinterpret_cast<GeneralWidget *>(widget), nullptr, event,
                                      first, second);
    }
}
} // namespace nfs::gui::widgets::family
