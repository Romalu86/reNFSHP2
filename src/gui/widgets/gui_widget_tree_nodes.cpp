#include "gui_widget_tree.hpp"
namespace nfs::gui::widgets::family {

static int node_index(Container *items, TreeNode *node) {
    if (node) {
        const Dword count = items->count;
        Element **entries = items->elements;
        for (Dword index = 0; index < count; ++index)
            if (entries[index] && entries[index]->value == node)
                return int(index);
    }
    return -1;
}

void set_node_selection(TreeNode *node, Dword selected) {
    if (!selected) {
        for (int index = 0; index < int(node->children.count); ++index)
            if (TreeNode *child = tree_child(node, Dword(index))) {
                set_node_selection(child, 0);
                clear_descendant_selection(child);
            }
        node->flags &= ~Dword(1);
    } else
        node->flags |= 1;
}
// Both recursive traversals are present in the body.
void clear_descendant_selection(TreeNode *node) {
    for (int index = 0; index < int(node->children.count); ++index)
        if (TreeNode *child = tree_child(node, Dword(index))) {
            clear_descendant_selection(child);
            child->flags &= ~Dword(1);
            clear_descendant_selection(child);
        }
}

TreeNode *previous_node_sibling(TreeNode *node) {
    if (TreeNode *parent = node_parent(node)) {
        const int index = node_index(&parent->children, node);
        if (index > 0)
            if (TreeNode *previous = tree_child(parent, Dword(index - 1)))
                return previous;
    }
    return nullptr;
}

TreeNode *last_expanded_descendant(TreeNode *node) {
    Dword count = node->children.count;
    if (count)
        while ((node->flags & 2) && count - 1 < node->children.count) {
            node = tree_child(node, count - 1);
            if (!node)
                return nullptr;
            count = node->children.count;
            if (!count)
                return node;
        }
    return node;
}

TreeNode *next_visible_node(TreeNode *node) {
    if ((node->flags & 2) && node->children.count)
        if (TreeNode *first = tree_child(node, 0))
            return first;
    for (TreeNode *current = node; node_parent(current); current = node_parent(current)) {
        TreeNode *parent = node_parent(current);
        const int index = node_index(&parent->children, current);
        if (index >= 0)
            if (TreeNode *next = tree_child(parent, Dword(index + 1)))
                return next;
    }
    return nullptr;
}

TreeNode *previous_visible_node(TreeNode *node) {
    if (TreeNode *previous = previous_node_sibling(node)) {
        TreeNode *last = previous;
        if (previous->children.count && (previous->flags & 2))
            if (TreeNode *child = tree_child(previous, previous->children.count - 1))
                last = last_expanded_descendant(child);
        return last ? last : previous;
    }
    return node_parent(node);
}

int node_depth(TreeNode *node) {
    return node_parent(node) ? node_depth(node_parent(node)) + 1 : 0;
}

int node_visible_descendants(TreeNode *node) {
    int count = 0;
    if (node->flags & 2)
        for (int index = 0; index < int(node->children.count); ++index)
            if (TreeNode *child = tree_child(node, Dword(index)))
                count += 1 + node_visible_descendants(child);
    return count;
}

void update_node_indices(TreeNode *node) {
    node->visible_descendants = Dword(node_visible_descendants(node));
    int index = 0;
    TreeNode *previous = previous_node_sibling(node);
    if (!previous)
        previous = node_parent(node);
    else if (TreeNode *last = last_expanded_descendant(previous))
        previous = last;
    while (previous) {
        ++index;
        TreeNode *sibling = previous_node_sibling(previous);
        if (!sibling)
            previous = node_parent(previous);
        else {
            TreeNode *last = last_expanded_descendant(sibling);
            previous = last ? last : sibling;
        }
    }
    node->visible_index = Dword(index);
    node->depth = node_parent(node) ? Dword(node_depth(node_parent(node)) + 1) : 0;
    const int count = int(node->children.count);
    for (int child = 0; child < count; ++child)
        if (TreeNode *entry = tree_child(node, Dword(child)))
            update_node_indices(entry);
}
// The optimized entry inlines the same traversal.
TreeNode *tree_row(Widget *widget, int row) {
    if (row < 0)
        return nullptr;
    TreeNode *node = &tree(widget)->root;
    while (row > 0) {
        --row;
        node = next_visible_node(node);
        if (!node)
            return nullptr;
    }
    return node;
}

Dword select_tree_row(Widget *widget, int row, Dword selected) {
    TreeNode *node = tree_row(widget, row);
    if (!node || (node->flags & 4) || (node->flags & 1) == selected)
        return 0;
    set_node_selection(node, selected);
    return 1;
}

Dword select_tree_range(Widget *widget, int first, int last) {
    if (!(tree(widget)->style & 1))
        return 0;
    if (first < 0) {
        TreeNode *node = tree_row(widget, last);
        const Dword previous = node ? node->flags & 1 : 0;
        node = tree_row(widget, last);
        if (!node || (node->flags & 4))
            return 0;
        set_node_selection(&tree(widget)->root, 0);
        clear_descendant_selection(&tree(widget)->root);
        set_node_selection(node, 1);
        return previous != 1;
    }
    Dword changed = 0;
    const auto deselect = [&](int row) {
        TreeNode *node = tree_row(widget, row);
        if (!node || (node->flags & 4) || !(node->flags & 1))
            return Dword(0);
        set_node_selection(node, 0);
        return Dword(1);
    };
    if (last <= first) {
        for (int row = 0; row < last; ++row)
            changed += deselect(row);
        for (int row = first + 1; row < int(tree(widget)->root.visible_descendants) + 1; ++row)
            changed += deselect(row);
        do {
            changed += select_tree_row(widget, first, 1);
            --first;
        } while (last <= first);
    } else {
        for (int row = 0; row < first; ++row)
            changed += deselect(row);
        for (int row = last + 1; row <= int(tree(widget)->root.visible_descendants) + 1; ++row)
            changed += deselect(row);
        do {
            changed += select_tree_row(widget, first, 1);
            ++first;
        } while (first <= last);
    }
    return changed;
}
} // namespace nfs::gui::widgets::family
