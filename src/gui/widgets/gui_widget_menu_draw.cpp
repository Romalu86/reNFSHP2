#include "gui_widget_menu.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::invoke;

Dword __fastcall menu_item_colour(Widget *widget, void *, engine::Engine *render, Dword index) {
    const auto *item = menu_item(menu(widget), index);
    const Dword entry = item->flags & 4 ? 2 : Dword(menu(widget)->selected) == index ? 3 : 0;
    Dword colour;
    return *invoke<Dword *>(render, 23, &colour, entry);
}

void __fastcall draw_menu_item(Widget *widget, void *, engine::Engine *render, Dword index,
                               const Rectangle *bounds, const Dword *colour) {
    auto *item = menu_item(menu(widget), index);
    if ((item->flags & 12) == 12) {
        Dword shadow, highlight;
        invoke<Dword *>(render, 23, &shadow, Dword(7));
        invoke<Dword *>(render, 23, &highlight, Dword(6));
        const int inset = invoke<int>(render, 49, Dword(1));
        Point begin{engine::point_methods, bounds->left, bounds->top};
        Point end{engine::point_methods, bounds->left + bounds->width, bounds->top};
        begin.y += inset;
        end.y += inset;
        invoke(render, 46, &begin, &end, shadow, shadow);
        begin.y += inset;
        end.y += inset;
        invoke(render, 46, &begin, &end, highlight, highlight);
        engine::destroy_point(&end);
        engine::destroy_point(&begin);
    } else
        invoke(render, 42, &item->text, bounds->left + menu(widget)->left_margin, bounds->top,
               colour, Dword(0));
}

void __fastcall menu_draw(Widget *widget, void *, Dword render_address) {
    auto *render = reinterpret_cast<engine::Engine *>(render_address);
    const Rectangle *viewport = engine::current_viewport_bounds(&render->base);
    Rectangle clip = *invoke<const Rectangle *>(render, 66);
    clip.methods = engine::rectangle_methods;
    engine::intersect_in_place(&clip, viewport);
    invoke(render, 64, &clip);
    engine::destroy_rectangle(&clip);
    using SetStyle = Dword(__thiscall *)(engine::Engine *, Dword);
    using Fill = void(__thiscall *)(engine::Engine *, const Rectangle *, const Dword *, Dword);
    const Method *palette_methods = render->base.methods;
    const Dword palette = inherited_palette(widget);
    reinterpret_cast<SetStyle>(palette_methods[21])(render, palette);
    Menu *owner = menu(widget);
    const int count = int(owner->items.count);
    for (int index = 0; index < count; ++index) {
        const Method *font_methods = render->base.methods;
        const Dword font = invoke<Dword>(widget, 51, render, Dword(index));
        reinterpret_cast<SetStyle>(font_methods[14])(render, font);
        if (auto *item = menu_item(owner, Dword(index))) {
            Point origin{engine::point_methods, viewport->left, viewport->top};
            Rectangle row;
            engine::translate_rectangle(&row, &item->bounds, &origin);
            engine::destroy_point(&origin);
            Dword colour = invoke<Dword>(widget, 50, render, Dword(index));
            if (owner->selected == index) {
                Dword selected;
                const Method *methods = render->base.methods;
                auto *selected_colour = invoke<Dword *>(render, 23, &selected, Dword(11));
                reinterpret_cast<Fill>(methods[48])(render, &row, selected_colour, Dword(1));
            }
            invoke(widget, 49, render, Dword(index), &row, &colour);
            const Dword image_handle = item->image;
            if (image_handle) {
                Rectangle image{engine::rectangle_methods, 0, 0, 0, 0};
                if (invoke<Dword>(render, 29, image_handle, &image, Dword(0))) {
                    Point offset{engine::point_methods, row.left, row.top};
                    Rectangle translated;
                    engine::translate_rectangle(&translated, &image, &offset);
                    engine::destroy_point(&offset);
                    Dword white = 4294967295u;
                    invoke(render, 45, image_handle, &translated, &white, Dword(0));
                    engine::destroy_rectangle(&translated);
                }
                engine::destroy_rectangle(&image);
            }
            if (owner->arrow.image && item->submenu) {
                Rectangle arrow{engine::rectangle_methods,
                                row.top + row.height / 2 - owner->arrow.bounds.height / 2,
                                row.left + row.width - owner->arrow.bounds.width,
                                owner->arrow.bounds.width, owner->arrow.bounds.height};
                invoke(render, 45, owner->arrow.image, &arrow, &colour, Dword(0));
                engine::destroy_rectangle(&arrow);
            }
            engine::destroy_rectangle(&row);
        }
    }
    invoke(render, 65);
}
} // namespace nfs::gui::widgets::family
