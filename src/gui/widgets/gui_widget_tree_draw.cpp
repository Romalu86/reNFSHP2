#include "gui_widget_tree.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::invoke;

void __fastcall draw_tree_expander(Widget *widget, void *, engine::Engine *render, TreeNode *node,
                                   const Point *centre) {
    auto *owner = tree(widget);
    if (!(owner->style & 8))
        return;
    const Dword flags = node->flags;
    Dword colour;
    invoke<Dword *>(render, 23, &colour, Dword(0));
    if (flags & 4)
        colour = *invoke<Dword *>(render, 23, &colour, Dword(2));
    const int size = owner->expand_icon_size, arm = size / 3;
    Rectangle bounds{engine::rectangle_methods, centre->y - size / 2, centre->x - size / 2, size,
                     size};
    Dword background;
    using Fill = void(__thiscall *)(engine::Engine *, const Rectangle *, const Dword *, Dword);
    const Method *background_methods = render->base.methods;
    Dword *background_colour = invoke<Dword *>(render, 23, &background, Dword(4));
    reinterpret_cast<Fill>(background_methods[48])(render, &bounds, background_colour, Dword(1));
    const int horizontal_x = int(Dword(bounds.left) + Dword(bounds.width / 2));
    const int horizontal_y = int(Dword(bounds.top) + Dword(bounds.height / 2));
    Point left{engine::point_methods, horizontal_x - arm, horizontal_y};
    Point right{engine::point_methods, horizontal_x + arm, horizontal_y};
    invoke(render, 46, &left, &right, colour, colour);
    if (!(flags & 2)) {
        const int vertical_x = int(Dword(bounds.left) + Dword(bounds.width / 2));
        const int vertical_y = int(Dword(bounds.top) + Dword(bounds.height / 2));
        Point top{engine::point_methods, vertical_x, vertical_y - arm};
        Point bottom{engine::point_methods, vertical_x, vertical_y + arm};
        invoke(render, 46, &top, &bottom, colour, colour);
        engine::destroy_point(&bottom);
        engine::destroy_point(&top);
    }
    Dword border;
    const Method *border_methods = render->base.methods;
    Dword *border_colour = invoke<Dword *>(render, 23, &border, Dword(5));
    reinterpret_cast<Fill>(border_methods[48])(render, &bounds, border_colour, Dword(0));
    engine::destroy_point(&right);
    engine::destroy_point(&left);
    engine::destroy_rectangle(&bounds);
}

void __fastcall draw_tree_row(Widget *widget, void *, engine::Engine *render, TreeNode *node) {
    const Rectangle *viewport = engine::current_viewport_bounds(&render->base);
    Rectangle client;
    client_bounds(widget, &client);
    auto *owner = tree(widget);
    Point offset{engine::point_methods, int(Dword(owner->indent) * (node->depth + 1)),
                 int(Dword(owner->row_height) * node->visible_index)};
    Point position{engine::point_methods, offset.x + viewport->left, offset.y + viewport->top};
    TreeNode *previous = previous_node_sibling(node);
    if (!previous)
        previous = node_parent(node);
    if (previous && (owner->style & 2)) {
        Point right{engine::point_methods, widget->scroll.x + position.x,
                    (owner->row_height >> 1) + widget->scroll.y + position.y};
        Point left{engine::point_methods, right.x - owner->indent, right.y};
        Dword colour;
        invoke<Dword *>(render, 23, &colour, Dword(10));
        invoke(render, 46, &left, &right, colour, colour);
        right.y -= int(Dword(owner->row_height) * (node->visible_index - previous->visible_index));
        right.x -= owner->indent;
        invoke(render, 46, &left, &right, colour, colour);
        engine::destroy_point(&left);
        engine::destroy_point(&right);
    }
    if (widget->scroll.y + owner->row_height + offset.y < 0 ||
        client.height < widget->scroll.y + offset.y || client.width < widget->scroll.x + offset.x) {
        engine::destroy_point(&position);
        engine::destroy_point(&offset);
        engine::destroy_rectangle(&client);
        return;
    }
    const Dword flags = node->flags;
    Dword palette = 2;
    if (!(flags & 4) && !(widget->flags & 1)) {
        palette = 0;
        if (flags & 1) {
            Rectangle selected{engine::rectangle_methods, position.y + widget->scroll.y,
                               position.x + widget->scroll.x, client.left + client.width - offset.x,
                               owner->row_height};
            invoke(widget, 47, render, &selected);
            engine::destroy_rectangle(&selected);
            palette = 3;
        }
    }
    Dword colour;
    colour = *invoke<Dword *>(render, 23, &colour, palette);
    invoke(render, 42, &node->text, widget->scroll.x + position.x, widget->scroll.y + position.y,
           &colour, Dword(0));
    engine::destroy_point(&position);
    engine::destroy_point(&offset);
    engine::destroy_rectangle(&client);
}

void __fastcall tree_draw(Widget *widget, void *, Dword render_address) {
    auto *render = reinterpret_cast<engine::Engine *>(render_address);
    invoke(render, 21, inherited_palette(widget));
    const Rectangle *viewport = engine::current_viewport_bounds(&render->base);
    Rectangle clip = *invoke<const Rectangle *>(render, 66);
    clip.methods = engine::rectangle_methods;
    engine::intersect_in_place(&clip, viewport);
    invoke(render, 64, &clip);
    engine::destroy_rectangle(&clip);
    auto *owner = tree(widget);
    const int start = first_visible_tree_row(widget);
    const int end = int(owner->root.visible_descendants) + 1;
    for (int row = start; row < end; ++row)
        if (TreeNode *node = tree_row(widget, row)) {
            if (!(owner->style & 4))
                invoke(widget, 48, render, node);
            else
                events::dispatch(widget, 72, render_address, reinterpret_cast<Dword>(node));
        }
    const int last = last_visible_tree_row(widget, visible_tree_rows(widget));
    for (int row = start; row < last; ++row)
        if (TreeNode *node = tree_row(widget, row);
            node && node->children.count && node_parent(node)) {
            Rectangle client;
            client_bounds(widget, &client);
            Point offset{engine::point_methods, int(Dword(owner->indent) * (node->depth + 1)),
                         int(Dword(owner->row_height) * node->visible_index)};
            Point position{engine::point_methods, offset.x + viewport->left,
                           offset.y + viewport->top};
            Point centre{engine::point_methods, widget->scroll.x + position.x - owner->indent,
                         (owner->row_height >> 1) + widget->scroll.y + position.y};
            invoke(widget, 50, render, node, &centre);
            engine::destroy_point(&centre);
            engine::destroy_point(&position);
            engine::destroy_point(&offset);
            engine::destroy_rectangle(&client);
        }
    invoke(render, 65);
}
} // namespace nfs::gui::widgets::family
