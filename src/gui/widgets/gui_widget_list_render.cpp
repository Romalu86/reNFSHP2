#include "gui_widget_controls.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;

Dword list_row_disabled(ListItems *items, Dword row) {
    const auto *item = invoke<const ListItem *>(items, 9, row, Dword(0));
    return item ? (item->flags >> 1) & 1 : 0;
}

Dword list_row_selected(ListItems *items, Dword row) {
    const auto *item = invoke<const ListItem *>(items, 9, row, Dword(0));
    return item ? item->flags & 1 : 0;
}

void __fastcall draw_list_selection(Widget *, void *, engine::Engine *render,
                                    const Rectangle *bounds) {
    Dword colour;
    const Method *methods = render->base.methods;
    Dword *selected = invoke<Dword *>(render, 23, &colour, Dword(11));
    using Fill = void(__thiscall *)(engine::Engine *, const Rectangle *, const Dword *, Dword);
    reinterpret_cast<Fill>(methods[48])(render, bounds, selected, Dword(1));
}

void __fastcall draw_list_column_divider(Widget *, void *, engine::Engine *render,
                                         const Point *first, const Point *second) {
    Dword colour;
    invoke(render, 23, &colour, Dword(10));
    invoke(render, 46, first, second, colour, colour);
}

void __fastcall draw_list_row_divider(Widget *, void *, engine::Engine *render, const Point *first,
                                      const Point *second) {
    Dword colour;
    invoke(render, 23, &colour, Dword(10));
    invoke(render, 46, first, second, colour, colour);
}

void __fastcall draw_list_header_background(Widget *widget, void *, engine::Engine *render,
                                            const Rectangle *bounds) {
    if (!(at<Dword>(widget, 320) & 1024))
        return;
    Dword colour;
    using Fill = void(__thiscall *)(engine::Engine *, const Rectangle *, const Dword *, Dword);
    const Method *background_methods = render->base.methods;
    Dword *background = invoke<Dword *>(render, 23, &colour, Dword(4));
    reinterpret_cast<Fill>(background_methods[48])(render, bounds, background, Dword(1));
    const Method *border_methods = render->base.methods;
    Dword *border = invoke<Dword *>(render, 23, &colour, Dword(5));
    reinterpret_cast<Fill>(border_methods[48])(render, bounds, border, Dword(0));
}
// Column is retained in the RET0Ch signature.
Dword __fastcall list_cell_colour(Widget *widget, void *, engine::Engine *render, Dword row, Dword) {
    auto *items = reinterpret_cast<ListItems *>(game::runtime::bytes(widget) + 264);
    Dword palette_index = 2;
    if (!(widget->flags & 1) && !list_row_disabled(items, row)) {
        if (!list_row_selected(items, row) || (at<Dword>(widget, 320) & 512))
            palette_index = 0;
        else if (row == at<Dword>(widget, 316) &&
                 ((widget->flags & 4096) || active_widget_id == widget->id))
            palette_index = 1;
        else
            palette_index = 3;
    }
    Dword colour;
    return *invoke<Dword *>(render, 23, &colour, palette_index);
}

void __fastcall draw_list_cell(Widget *widget, void *, engine::Engine *render, Dword row,
                               Dword column) {
    Widget *cell = invoke<Widget *>(widget, 47, column);
    auto *items = reinterpret_cast<ListItems *>(game::runtime::bytes(widget) + 264);
    auto *item = invoke<ListItem *>(items, 9, row, column);
    if (!item || !cell)
        return;
    invoke(cell, 7, &item->text);
    invoke(render, 21, inherited_palette(cell));
    const Dword colour = at<Dword>(widget, 320) & 256
                            ? events::dispatch(widget, 65, reinterpret_cast<Dword>(render),
                                               (column & 65535) << 16 | (row & 65535))
                            : invoke<Dword>(widget, 55, render, row, column);
    cell->colour = colour;
    engine::pending::draw_root(cell, render, 0);
    invoke(cell, 7, &lifecycle::construction::unresolved::shared_string);
}

void __fastcall draw_tree_selection(Widget *, void *, engine::Engine *render,
                                    const Rectangle *bounds) {
    Dword colour;
    const Method *methods = render->base.methods;
    Dword *selected = invoke<Dword *>(render, 23, &colour, Dword(11));
    using Fill = void(__thiscall *)(engine::Engine *, const Rectangle *, const Dword *, Dword);
    reinterpret_cast<Fill>(methods[48])(render, bounds, selected, Dword(1));
}

void __fastcall copy_menu_visual_settings(Widget *widget, void *, Dword source_address) {
    auto *source = reinterpret_cast<Widget *>(source_address);
    const Dword left_margin = at<Dword>(source, 268), right_margin = at<Dword>(source, 272);
    at<Dword>(widget, 264) = at<Dword>(source, 264);
    at<Dword>(widget, 268) = left_margin;
    at<Dword>(widget, 272) = right_margin;
    copy_image_resource(&at<ImageResource>(widget, 316), &at<ImageResource>(source, 316));
    const Dword overlap = at<Dword>(source, 284), delay_bits = at<Dword>(source, 288);
    at<Dword>(widget, 280) = at<Dword>(source, 280);
    const Dword menu_style = at<Dword>(source, 276);
    at<Dword>(widget, 284) = overlap;
    at<Dword>(widget, 288) = delay_bits;
    at<Dword>(widget, 276) = menu_style;
}
// The saved leaf ignores both supplied item arguments.
Dword __fastcall menu_item_font_style(Widget *widget, void *, Dword, Dword) {
    return inherited_font_style(widget);
}
} // namespace nfs::gui::widgets::family
