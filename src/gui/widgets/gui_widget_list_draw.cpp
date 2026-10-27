#include "gui_widget_list.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::invoke;
namespace {
int row_offset(Widget *widget, int row) {
    auto *list = list_box(widget);
    if (!(list->style & 32))
        return int(Dword(list->row_height) * Dword(row));
    int result = 0;
    for (int index = 0; index < row; ++index)
        result = int(Dword(result) + Dword(list_row_height(widget, index)));
    return result;
}
Rectangle clipped_row(const Rectangle *row, const Point *origin, const Rectangle *clip) {
    Rectangle translated;
    engine::translate_rectangle(&translated, row, origin);
    engine::intersect_in_place(&translated, clip);
    return translated;
}
} // namespace

void __fastcall list_box_draw(Widget *widget, void *, Dword render_address) {
    auto *render = reinterpret_cast<engine::Engine *>(render_address);
    const Rectangle *viewport = engine::current_viewport_bounds(&render->base);
    Rectangle clip = *invoke<const Rectangle *>(render, 66);
    clip.methods = engine::rectangle_methods;
    engine::intersect_in_place(&clip, viewport);
    const int first = first_visible_list_row(widget) - 1;
    const int start = first < 0 ? 0 : first;
    const int visible = visible_list_rows(widget);
    const int end = last_visible_list_row(widget, visible) + 1;
    auto *list = list_box(widget);
    const int columns = int(list->items.grid.columns.count);
    Point origin{engine::point_methods, viewport->left + widget->scroll.x,
                 viewport->top + widget->scroll.y};
    invoke(render, 21, inherited_palette(widget));
    invoke(widget, 54, render, viewport);
    for (int row = start; row <= end; ++row) {
        const auto *item = invoke<ListItem *>(&list->items, 9, row, Dword(0));
        if (!item)
            continue;
        const int height = list_row_height(widget, row);
        Rectangle bounds{engine::rectangle_methods, row_offset(widget, row), 0,
                         widget->bounds.width, height};
        const Dword flags = item->flags;
        if (flags & 4) {
            Rectangle clipped = clipped_row(&bounds, &origin, &clip);
            Point left{engine::point_methods, clipped.left, clipped.top + clipped.height};
            Point right{engine::point_methods, clipped.left + clipped.width,
                        clipped.top + clipped.height};
            invoke(widget, 52, render, &left, &right);
            engine::destroy_point(&right);
            engine::destroy_point(&left);
            engine::destroy_rectangle(&clipped);
        }
        if ((flags & 1) && !(list->style & 512)) {
            Rectangle clipped = clipped_row(&bounds, &origin, &clip);
            invoke(widget, 51, render, &clipped);
            engine::destroy_rectangle(&clipped);
        }
        engine::destroy_rectangle(&bounds);
    }
    for (int column = 0; column < columns; ++column) {
        auto *cell = invoke<Widget *>(widget, 47, column);
        if (!cell)
            continue;
        cell->flags &= ~Dword(2);
        const Rectangle saved{engine::rectangle_methods, cell->bounds.top, cell->bounds.left,
                              cell->bounds.width, cell->bounds.height};
        int top = row_offset(widget, start);
        for (int row = start; row <= end; ++row) {
            Rectangle positioned{engine::rectangle_methods, saved.top + top, saved.left,
                                 saved.width, saved.height};
            engine::copy_rectangle_fields(&cell->bounds, &positioned);
            if (!(list->style & 256))
                invoke(widget, 56, render, row, column);
            else
                events::dispatch(widget, 66, render_address,
                                 Dword(column) << 16 | (Dword(row) & 65535));
            top = int(Dword(top) + Dword(list_row_height(widget, row)));
            engine::destroy_rectangle(&positioned);
        }
        engine::copy_rectangle_fields(&cell->bounds, &saved);
        cell->flags |= 2;
    }
    if (list->style & 2)
        for (int column = 0; column < columns; ++column) {
            Widget *checked = invoke<Widget *>(widget, 48, column);
            Widget *unchecked = invoke<Widget *>(widget, 49, column);
            if (checked)
                for (int row = start; row <= end; ++row)
                    if (auto *item = invoke<ListItem *>(&list->items, 9, row, column);
                        item && (item->flags & 8)) {
                        checked->flags &= ~Dword(2);
                        checked->bounds.top += row_offset(widget, row);
                        engine::pending::draw_root(checked, render, 0);
                        checked->bounds.top -= row_offset(widget, row);
                        checked->flags |= 2;
                    }
            if (unchecked)
                for (int row = start; row <= end; ++row)
                    if (auto *item = invoke<ListItem *>(&list->items, 9, row, column);
                        item && !(item->flags & 8)) {
                        unchecked->flags &= ~Dword(2);
                        unchecked->bounds.top += row_offset(widget, row);
                        engine::pending::draw_root(unchecked, render, 0);
                        unchecked->bounds.top -= row_offset(widget, row);
                        unchecked->flags |= 2;
                    }
        }
    if (list->style & 4) {
        Rectangle client;
        client_bounds(widget, &client);
        invoke(render, 21, inherited_palette(widget));
        for (Dword column = 0; column < 32; ++column)
            if (Widget *cell = invoke<Widget *>(widget, 47, column)) {
                const Rectangle bounds{engine::rectangle_methods, cell->bounds.top,
                                       cell->bounds.left, cell->bounds.width, cell->bounds.height};
                Point upper{engine::point_methods, bounds.left + bounds.width + viewport->left,
                            viewport->top};
                Point lower{engine::point_methods, bounds.left + bounds.width + viewport->left,
                            client.height + viewport->top};
                if (clip.left < upper.x && upper.x < clip.left + clip.width)
                    invoke(widget, 53, render, &upper, &lower);
                engine::destroy_point(&lower);
                engine::destroy_point(&upper);
            }
        engine::destroy_rectangle(&client);
    }
    engine::destroy_point(&origin);
    engine::destroy_rectangle(&clip);
}
} // namespace nfs::gui::widgets::family
