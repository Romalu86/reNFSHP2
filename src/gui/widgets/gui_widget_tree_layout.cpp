#include "gui_widget_tree.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;
using kernel::numeric::Extended;

int first_visible_tree_row(Widget *widget) {
    auto *owner = tree(widget);
    return owner->row_height ? clamp_gui_integer(-(widget->scroll.y / owner->row_height), 0,
                                                 int(owner->root.visible_descendants))
                             : -1;
}

int visible_tree_rows(Widget *widget) {
    Rectangle bounds;
    client_bounds(widget, &bounds);
    const int height = tree(widget)->row_height;
    const int rows = height && bounds.height ? bounds.height / height : 0;
    engine::destroy_rectangle(&bounds);
    return rows;
}
int last_visible_tree_row(Widget *widget, int visible) {
    return tree(widget)->row_height && visible
               ? clamp_gui_integer(first_visible_tree_row(widget) - 1 + visible, 0,
                                   int(tree(widget)->root.visible_descendants))
               : -1;
}

void ensure_tree_row_visible(Widget *widget, int row) {
    auto *owner = tree(widget);
    if (int(owner->root.visible_descendants) + 1 <= 0)
        return;
    const int saved_y = widget->scroll.y;
    row = clamp_gui_integer(row, 0, int(owner->root.visible_descendants));
    int boundary = first_visible_tree_row(widget);
    const int height = owner->row_height;
    Rectangle client;
    client_bounds(widget, &client);
    const int visible = height && client.height ? client.height / height : 0;
    const int last = last_visible_tree_row(widget, visible);
    if (row < boundary || (boundary = last, last < row))
        widget->scroll.y = int(Dword(saved_y) - (Dword(row) - Dword(boundary)) * Dword(height));
    engine::destroy_rectangle(&client);
}

Dword __fastcall tree_scroll_command(Widget *widget, void *, Dword command, Dword position) {
    auto *owner = tree(widget);
    int row;
    switch (command) {
    case 0:
        row = first_visible_tree_row(widget) - 1;
        break;
    case 1:
        row = last_visible_tree_row(widget, visible_tree_rows(widget)) + 1;
        break;
    case 2:
        row = first_visible_tree_row(widget) - visible_tree_rows(widget);
        break;
    case 3: {
        const int visible = visible_tree_rows(widget);
        row = last_visible_tree_row(widget, visible) + visible_tree_rows(widget);
        break;
    }
    case 4:
        widget->scroll.y = int(Dword(0) - Dword(owner->row_height) * position);
        return 1;
    case 5:
        row = 0;
        break;
    case 6:
        row = int(owner->root.visible_descendants);
        break;
    default:
        return 1;
    }
    ensure_tree_row_visible(widget, row);
    return 1;
}

void __fastcall tree_scrollbar(Widget *widget, void *, Dword address) {
    auto *bar = reinterpret_cast<Widget *>(address);
    if (!(at<Dword>(bar, 264) & 2)) {
        const int last_row = int(tree(widget)->root.visible_descendants);
        const int count = last_row + 1;
        if (!count) {
            set_enabled(bar, 0);
            application::pending::set_widget_visible(bar, 0);
            notify_scroll(widget, nullptr, bar);
            return;
        }
        const int first = first_visible_tree_row(widget);
        const int last = last_visible_tree_row(widget, visible_tree_rows(widget));
        const int visible = visible_tree_rows(widget);
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
            set_enabled(plus, last < last_row);
        if (Widget *minus = find_child(bar, at<Dword>(bar, 324)))
            set_enabled(minus, 0 < first);
    }
    notify_scroll(widget, nullptr, bar);
}

void __fastcall tree_accept(Widget *widget, void *) {
    auto *owner = tree(widget);
    const int count = int(owner->root.visible_descendants) + 1;
    int selected = -1;
    for (int row = 0; row < count; ++row)
        if (auto *node = tree_row(widget, row); node && (node->flags & 1)) {
            selected = row;
            break;
        }
    TreeNode *node = tree_row(widget, selected);
    if (!node || !node->children.count || (node->flags & 4))
        return;
    const Dword flags = node->flags;
    if (flags & 2) {
        for (int index = 0; index < int(node->children.count); ++index)
            if (TreeNode *child = tree_child(node, Dword(index))) {
                clear_descendant_selection(child);
                child->flags &= ~Dword(1);
                clear_descendant_selection(child);
            }
        node->flags = flags & ~Dword(2);
    } else
        node->flags = flags | 2;
    owner->root.visible_descendants = Dword(node_visible_descendants(&owner->root));
    int index = 0;
    for (TreeNode *previous = previous_visible_node(&owner->root); previous;
         previous = previous_visible_node(previous))
        ++index;
    owner->root.visible_index = Dword(index);
    owner->root.depth =
        node_parent(&owner->root) ? Dword(node_depth(node_parent(&owner->root)) + 1) : 0;
    const int children = int(owner->root.children.count);
    for (int child = 0; child < children; ++child)
        if (TreeNode *entry = tree_child(&owner->root, Dword(child)))
            update_node_indices(entry);
}
} // namespace nfs::gui::widgets::family
