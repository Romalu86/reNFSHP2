#include "gui_widget_image_list.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::widgets::family {

int wrap_image_cell(Widget *widget, int index) {
    const int count = int(image_list(widget)->items.count);
    if (!count)
        return 0;
    const int wrapped = index % count;
    return wrapped < 0 ? wrapped + count : wrapped;
}
ImageItem *image_item_at(Widget *widget, int index) {
    auto &items = image_list(widget)->items;
    if (Dword(index) >= items.count)
        return nullptr;
    auto *element = items.elements[index];
    return element ? static_cast<ImageItem *>(element->value) : nullptr;
}

Dword image_cell_id(Widget *widget, int index) {
    auto *item = image_item_at(widget, wrap_image_cell(widget, index));
    return item ? item->id : 0;
}
// The caller may push an ignored argument; RET is plain.
int visible_image_cells(Widget *widget) {
    Rectangle client;
    client_bounds(widget, &client);
    const int width = image_list(widget)->cell_size.x;
    const int count = width && client.width ? client.width / width + 1 : 0;
    engine::destroy_rectangle(&client);
    return count;
}

int first_image_cell(Widget *widget) {
    const int width = image_list(widget)->cell_size.x;
    return width ? -1 - widget->scroll.x / width : -1;
}

int last_image_cell(Widget *widget) {
    const int count = visible_image_cells(widget);
    return count ? first_image_cell(widget) + 1 + count : -1;
}

Dword image_cell_visible(Widget *widget, int index) {
    index = wrap_image_cell(widget, index);
    const int first = wrap_image_cell(widget, first_image_cell(widget));
    const int last = wrap_image_cell(widget, last_image_cell(widget));
    return last < first ? index <= last || first <= index : first <= index && index <= last;
}
void refresh_image_frames(Widget *widget) {
    for (int index = 0; index <= int(image_list(widget)->items.count); ++index)
        if (auto *item = image_item_at(widget, index)) {
            if (image_cell_visible(widget, index) || image_cell_visible(widget, index - 1) ||
                image_cell_visible(widget, index + 1))
                ensure_image_item_frame(item, &image_list(widget)->sequence);
            else
                release_image_item_frame(item);
        }
}

void ensure_image_cell_visible(Widget *widget, int index) {
    auto *list = image_list(widget);
    if (int(list->items.count) < 1)
        return;
    const int left = widget->scroll.x + list->cell_size.x * index;
    if (left < 0)
        widget->scroll.x = -list->cell_size.x * index;
    else if (left + list->cell_size.x > widget->bounds.width)
        widget->scroll.x = widget->bounds.width - (index + 1) * list->cell_size.x;
    refresh_image_frames(widget);
}

Point *image_scroll_delta(Widget *widget, Point *out, int index) {
    *out = Point{engine::point_methods,
                 (image_list(widget)->center_cell - index) * image_list(widget)->cell_size.x -
                     widget->scroll.x,
                 0};
    return out;
}

int next_enabled_image_cell(Widget *widget, int index) {
    const int next = index + 1;
    if (image_list(widget)->style & 8)
        return next;
    int candidate = next;
    const int count = int(image_list(widget)->items.count);
    for (int checked = 0; checked < count; ++checked, ++candidate)
        if (image_item_enabled(image_cell_id(widget, candidate)))
            return candidate;
    return index;
}

int previous_enabled_image_cell(Widget *widget, int index) {
    if (image_list(widget)->style & 8)
        return index - 1;
    int candidate = index;
    for (int checked = 0; checked < int(image_list(widget)->items.count); ++checked) {
        --candidate;
        if (image_item_enabled(image_cell_id(widget, candidate)))
            return candidate;
    }
    return index;
}

Dword select_image_cell(Widget *widget, int index, Dword ensure_visible) {
    auto *list = image_list(widget);
    if (!list->items.count ||
        (!(list->style & 8) && !image_item_enabled(image_cell_id(widget, index))))
        return 0;
    const bool changed = wrap_image_cell(widget, index) != wrap_image_cell(widget, list->selected);
    list->selected = index;
    if (ensure_visible)
        ensure_image_cell_visible(widget, index);
    return changed;
}
} // namespace nfs::gui::widgets::family
