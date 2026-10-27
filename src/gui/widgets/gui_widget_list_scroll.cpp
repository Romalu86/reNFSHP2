#include "gui_widget_items.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <algorithm>
namespace nfs::gui::widgets {
Rectangle *client_bounds(Widget *, Rectangle *);
}
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
static ListItems *items_of(Widget *widget) {
    return reinterpret_cast<ListItems *>(bytes(widget) + 264);
}

int list_row_height(Widget *widget, int row) {
    if (row >= 0 && row < at<int>(widget, 280) && (at<Dword>(widget, 320) & 32))
        if (auto *item = invoke<ListItem *>(items_of(widget), 9, row, Dword(0)))
            if (item->image)
                return int(item->image);
    return at<int>(widget, 308);
}

int first_visible_list_row(Widget *widget) {
    const int offset = widget->scroll.y;
    int row = 0;
    if (!(at<Dword>(widget, 320) & 32)) {
        const int height = at<int>(widget, 308);
        if (!height)
            return -1;
        row = int(Dword(0) - Dword(offset / height));
    } else {
        const int count = at<int>(widget, 280);
        int distance = 0;
        while (row < count) {
            if (int(Dword(0) - Dword(offset)) <= distance)
                break;
            distance = int(Dword(distance) + Dword(list_row_height(widget, row)));
            ++row;
        }
    }
    const int last = int(at<Dword>(widget, 280) - 1);
    return std::min(std::max(row, std::min(last, 0)), last);
}

int visible_list_rows(Widget *widget) {
    Rectangle bounds;
    client_bounds(widget, &bounds);
    int result = 0;
    try {
        if (!(at<Dword>(widget, 320) & 32)) {
            const int height = at<int>(widget, 308);
            if (height && bounds.height)
                result = bounds.height / height;
        } else if (bounds.height) {
            const int count = at<int>(widget, 280);
            int row = first_visible_list_row(widget), distance = 0;
            while (row < count) {
                distance = int(Dword(distance) + Dword(list_row_height(widget, row)));
                if (bounds.height <= distance)
                    break;
                ++row;
                ++result;
            }
        }
    } catch (...) {
        engine::destroy_rectangle(&bounds);
        throw;
    }
    engine::destroy_rectangle(&bounds);
    return result;
}
// The two call sites inline this same first/count/clamp sequence.
int last_visible_list_row(Widget *widget, int visible) {
    if (!visible)
        return -1;
    const int row = int(Dword(first_visible_list_row(widget)) - 1 + Dword(visible));
    const int last = int(at<Dword>(widget, 280) - 1);
    return std::min(std::max(row, std::min(last, 0)), last);
}

void ensure_list_box_row_visible(Widget *widget, int row) {
    if (at<int>(widget, 280) < 1)
        return;
    int x = widget->scroll.x, y = widget->scroll.y;
    const int last = int(at<Dword>(widget, 280) - 1);
    row = std::min(std::max(row, std::min(last, 0)), last);
    int first = first_visible_list_row(widget);
    int bottom = last_visible_list_row(widget, visible_list_rows(widget));
    if (row < first) {
        bottom = first;
        if (at<Dword>(widget, 320) & 32) {
            do {
                y = int(Dword(y) + Dword(list_row_height(widget, row)));
                ++row;
            } while (row < first);
            widget->scroll.x = x;
            widget->scroll.y = y;
            return;
        }
    } else {
        if (row <= bottom)
            return;
        if (at<Dword>(widget, 320) & 32) {
            do {
                x = widget->scroll.x;
                y = widget->scroll.y;
                y = int(Dword(y) - Dword(list_row_height(widget, first)));
                widget->scroll.x = x;
                widget->scroll.y = y;
                bottom = last_visible_list_row(widget, visible_list_rows(widget));
                ++first;
            } while (bottom < row);
            widget->scroll.x = x;
            widget->scroll.y = y;
            return;
        }
    }
    y = int(Dword(y) - Dword(at<int>(widget, 308)) * (Dword(row) - Dword(bottom)));
    widget->scroll.x = x;
    widget->scroll.y = y;
}

void __fastcall erase_list_box_row(ListItems *items, void *, Dword row) {
    grid_remove_row(&items->grid, row);
    auto *widget = reinterpret_cast<Widget *>(bytes(items) - 264);
    const int count = int(items->grid.rows.count);
    if (!count) {
        widget->scroll.x = engine::default_point.x;
        widget->scroll.y = engine::default_point.y;
        return;
    }
    const int x = widget->scroll.x, y = widget->scroll.y;
    const int last = int(Dword(count) - 1);
    int distance = 0;
    if (!(at<Dword>(widget, 320) & 32))
        distance = int(Dword(at<int>(widget, 308)) * Dword(last));
    else
        for (int index = 0; index < last; ++index)
            distance = int(Dword(distance) + Dword(list_row_height(widget, index)));
    if (int(Dword(list_row_height(widget, last)) + Dword(distance) + Dword(y)) <
        widget->bounds.height) {
        const int visible = visible_list_rows(widget);
        const int first = std::max(int(Dword(count) - Dword(visible)), 0);
        if (!(at<Dword>(widget, 320) & 32))
            distance = int(Dword(at<int>(widget, 308)) * Dword(first));
        else {
            distance = 0;
            for (int index = 0; index < first; ++index)
                distance = int(Dword(distance) + Dword(list_row_height(widget, index)));
        }
        widget->scroll.y = int(Dword(0) - Dword(distance));
        widget->scroll.x = x;
    }
}
} // namespace nfs::gui::widgets::family
