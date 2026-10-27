#include "frontend_menu_roots.hpp"
#include "../../gui/widgets/gui_widget_list.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;

Dword selected_menu_image(Widget *widget) {
    auto *item = family::image_item_at(
        widget, family::wrap_image_cell(widget, family::image_list(widget)->selected));
    return item ? item->id : 0;
}

Dword menu_image_value(Dword id) {
    auto *item = id ? family::find_image_item(id) : nullptr;
    return item ? item->data : 0;
}

int menu_image_index(Widget *widget, Dword id) {
    for (Dword i = 0; i < family::image_list(widget)->items.count; ++i)
        if (auto *item = family::image_item_at(widget, int(i)))
            if (item->id == id)
                return int(i);
    return -1;
}

void copy_menu_point(gui::engine::Point *point, const gui::engine::Point *source) {
    const int x = source->x, y = source->y;
    point->methods = gui::engine::point_methods;
    point->x = x;
    point->y = y;
}

void align_menu_image_list(Widget *widget) {
    auto *list = family::image_list(widget);
    gui::engine::Point delta;
    family::image_scroll_delta(widget, &delta, list->selected);
    if (delta.x || delta.y) {
        widget->scroll.x = delta.x;
        widget->scroll.y = delta.y;
        gui::engine::Point copied;
        copy_menu_point(&copied, &widget->scroll);
        if (family::visible_image_cells(widget))
            (void)family::first_image_cell(widget);
        family::refresh_image_frames(widget);
        gui::events::post_widget(widget, 56, 0, 0);
    }
    delta.methods = gui::base_methods;
}

void refresh_menu_image_navigation(MenuImageList *list) {
    list->reserved = 0;
    list->navigation_count = 0;
    auto *widget = reinterpret_cast<Widget *>(list);
    for (int i = 0; i < int(list->base.items.count); ++i)
        if (family::image_item_enabled(family::image_cell_id(widget, i)))
            ++list->navigation_count;
        else
            ++list->reserved;
}

gui::engine::Point *menu_rectangle_center(const gui::engine::Rectangle *rectangle,
                                          gui::engine::Point *point) {
    const int y = int(Dword(rectangle->height / 2) + Dword(rectangle->top));
    const int x = int(Dword(rectangle->width / 2) + Dword(rectangle->left));
    point->methods = gui::engine::point_methods;
    point->x = x;
    point->y = y;
    return point;
}

float clamp_car_zoom(float value, float lower, float upper) {
    if (value < lower)
        return lower;
    if (upper < value)
        return upper;
    return value;
}
} // namespace nfs::application::frontend
