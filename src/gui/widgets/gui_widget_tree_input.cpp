#include "gui_widget_tree.hpp"
#include "nfs/core/application/application.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::invoke;
namespace {
Dword selected_tree_row(Widget *widget, int row) {
    TreeNode *node = tree_row(widget, row);
    return node ? node->flags & 1 : 0;
}
Dword select_single_node(Widget *widget, int row, Dword selected) {
    const Dword previous = selected_tree_row(widget, row);
    TreeNode *node = tree_row(widget, row);
    if (!node || (node->flags & 4))
        return 0;
    set_node_selection(&tree(widget)->root, 0);
    clear_descendant_selection(&tree(widget)->root);
    set_node_selection(node, selected);
    return previous != selected;
}
int previous_enabled_tree_row(Widget *widget, int row) {
    do {
        --row;
        if (row < 0)
            return -1;
        if (TreeNode *node = tree_row(widget, row); node && !(node->flags & 4))
            return row;
    } while (true);
}
int next_enabled_tree_row(Widget *widget, int row) {
    const int end = int(tree(widget)->root.visible_descendants) + 1;
    do {
        ++row;
        if (row >= end)
            return -1;
        if (TreeNode *node = tree_row(widget, row); node && !(node->flags & 4))
            return row;
    } while (true);
}
} // namespace

void __fastcall tree_button_down(Widget *widget, void *, Dword button, Dword modifiers) {
    if (button == 7 || button == 3 || button == 4)
        invoke<Dword>(widget, 32, Dword(button == 7 ? 28 : button == 3 ? 208 : 200), modifiers);
    else
        pending::general_button(reinterpret_cast<GeneralWidget *>(widget), nullptr, button,
                                modifiers);
}

Dword __fastcall tree_key_down(Widget *widget, void *, Dword key, Dword modifiers) {
    auto *owner = tree(widget);
    switch (key) {
    case 1:
        release_capture(widget);
        break;
    case 28:
        if (!std::uint8_t(modifiers))
            events::post_widget(widget, 40, 0, 0);
        return 1;
    case 203:
        pending::previous_tab(widget);
        return 1;
    case 205:
        pending::next_tab(widget);
        return 1;
    case 200:
    case 201:
    case 208:
    case 209: {
        const int before = owner->cursor;
        const bool up = key == 200 || key == 201;
        if (up ? before < 1 : int(owner->root.visible_descendants) <= before) {
            if (key == 201)
                break;
            return 1;
        }
        int row;
        if (key == 200)
            row = previous_enabled_tree_row(widget, before);
        else if (key == 208)
            row = next_enabled_tree_row(widget, before);
        else {
            const int page = visible_tree_rows(widget);
            if (up) {
                row = previous_enabled_tree_row(widget, before - page);
                if (row < 0)
                    row = next_enabled_tree_row(widget, before - page);
            } else {
                row = next_enabled_tree_row(widget, before + page);
                if (row < 0)
                    row = previous_enabled_tree_row(widget, before + page);
            }
        }
        const bool in_direction = key == 201 ? row < before : key == 209 ? before < row : true;
        if (in_direction && row >= 0 && row < int(owner->root.visible_descendants) + 1) {
            owner->cursor = row;
            ensure_tree_row_visible(widget, row);
        }
        Dword changed;
        if ((owner->style & 1) && (modifiers & 196608))
            changed = select_tree_range(widget, owner->anchor, owner->cursor);
        else {
            changed = select_single_node(widget, owner->cursor, 1);
            if (changed && owner->cursor >= 0 &&
                owner->cursor < int(owner->root.visible_descendants) + 1)
                owner->anchor = owner->cursor;
        }
        if (changed)
            events::post_widget(widget, 73, 0, 0);
        if (key == 201)
            break;
        return 1;
    }
    }
    return pending::general_key(reinterpret_cast<GeneralWidget *>(widget), nullptr, key, modifiers);
}

Dword __fastcall tree_pointer_press(Widget *widget, void *, Dword button, Dword modifiers) {
    auto *owner = tree(widget);
    if (!owner->row_height)
        return pending::update_focus_style(widget, nullptr, button, modifiers);
    Point local;
    from_screen_point(widget, &local, pending::pointer_position());
    if (local.y < 0 || widget->bounds.height < local.y) {
        if (application::pending::captured_widget_id == widget->id) {
            int row = 0;
            bool scroll = false;
            if (widget->bounds.height < local.y &&
                owner->cursor < int(owner->root.visible_descendants)) {
                row = last_visible_tree_row(widget, visible_tree_rows(widget)) + 1;
                scroll = true;
            } else if (local.y < 0 && owner->cursor >= 1) {
                row = first_visible_tree_row(widget) - 1;
                scroll = true;
            }
            if (scroll)
                ensure_tree_row_visible(widget, row);
        }
    } else {
        const int row = local.y / owner->row_height + first_visible_tree_row(widget);
        if ((modifiers & 33554432) && row == owner->cursor) {
            events::post_widget(widget, 40, 0, 0);
            engine::destroy_point(&local);
            return 1;
        }
        capture_widget(widget);
        const bool multiple = (owner->style & 1) != 0;
        bool move = false;
        if (multiple && (modifiers & 786432)) {
            if (!std::uint8_t(modifiers)) {
                const Dword selected = selected_tree_row(widget, row);
                move = select_tree_row(widget, row, selected == 0) != 0;
            }
        } else if (multiple && (modifiers & 196608))
            move = select_tree_range(widget, owner->anchor, row) != 0;
        else if (multiple) {
            const Dword selected = selected_tree_row(widget, row);
            move = select_single_node(widget, row, selected == 0) != 0;
        } else if (!selected_tree_row(widget, row))
            move = select_single_node(widget, row, 1) != 0;
        if (move && row >= 0 && row < int(owner->root.visible_descendants) + 1) {
            owner->cursor = row;
            ensure_tree_row_visible(widget, row);
        }
    }
    activate(widget);
    engine::destroy_point(&local);
    return 1;
}

Dword __fastcall tree_pointer_release(Widget *widget, void *, Dword button, Dword modifiers) {
    if (!button && application::pending::captured_widget_id == widget->id) {
        const int count = int(tree(widget)->root.visible_descendants) + 1;
        for (int row = 0; row < count; ++row)
            if (TreeNode *node = tree_row(widget, row); node && (node->flags & 1)) {
                events::post_widget(widget, 73, 0, 0);
                break;
            }
        release_capture(widget);
    }
    return capture_style(widget, nullptr, button, modifiers);
}
} // namespace nfs::gui::widgets::family
