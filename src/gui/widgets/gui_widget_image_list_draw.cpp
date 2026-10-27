#include "gui_widget_image_list.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::invoke;

Dword __fastcall image_list_cell_colour(Widget *widget, void *, engine::Engine *render, Dword id) {
    auto *item = find_image_item(id);
    if (!item)
        return 0;
    Dword colour;
    const Dword palette = !item->enabled                                                    ? 2
                         : item->id == image_cell_id(widget, image_list(widget)->selected) ? 3
                                                                                           : 0;
    return *invoke<Dword *>(render, 23, &colour, palette);
}

void __fastcall draw_image_list_cell(Widget *, void *, engine::Engine *render, Dword id,
                                     const Rectangle *bounds, const Dword *colour) {
    auto *item = find_image_item(id);
    if (item && item->resource.image)
        invoke(render, 45, item->resource.image, bounds, colour, Dword(0));
}

void __fastcall image_list_draw(Widget *widget, void *, Dword render_address) {
    auto *list = image_list(widget);
    const int count = int(list->items.count);
    if (!count)
        return;
    auto *render = reinterpret_cast<engine::Engine *>(render_address);
    const Dword clip_cells = list->style & 1;
    Rectangle viewport = *engine::current_viewport_bounds(&render->base);
    viewport.methods = engine::rectangle_methods;
    invoke(render, 64, engine::current_viewport_bounds(&render->base));
    invoke(render, 21, inherited_palette(widget));
    int first = first_image_cell(widget);
    const int last = first + visible_image_cells(widget);
    if (last < first)
        first -= count;
    for (int index = first; index <= last; ++index) {
        auto *item = image_item_at(widget, wrap_image_cell(widget, index));
        if (!item)
            continue;
        Point offset{engine::point_methods, list->cell_size.x * index + widget->scroll.x, 0};
        Rectangle bounds;
        engine::translate_rectangle(&bounds, &viewport, &offset);
        engine::destroy_point(&offset);
        const Dword id = item->id;
        Dword colour = invoke<Dword>(widget, 50, render, id);
        bounds.width = list->image_size.x ? list->image_size.x : item->resource.bounds.width;
        bounds.height = list->image_size.y ? list->image_size.y : item->resource.bounds.height;
        bounds.top += (viewport.height - bounds.height) / 2;
        bounds.left += (list->cell_size.x - bounds.width) / 2;
        if (clip_cells) {
            Rectangle clip = bounds;
            clip.methods = engine::rectangle_methods;
            engine::intersect_in_place(&clip, engine::current_viewport_bounds(&render->base));
            invoke(render, 64, &clip);
            engine::destroy_rectangle(&clip);
        }
        if (list->style & 4) {
            bounds.width = item->resource.bounds.width;
            bounds.height = item->resource.bounds.height;
        }
        if (list->style & 2) {
            bounds.left += item->resource.bounds.left;
            bounds.top += item->resource.bounds.top;
        }
        invoke(widget, 49, render, id, &bounds, &colour);
        if (clip_cells)
            invoke(render, 65);
        engine::destroy_rectangle(&bounds);
    }
    invoke(render, 65);
    engine::destroy_rectangle(&viewport);
}
} // namespace nfs::gui::widgets::family
