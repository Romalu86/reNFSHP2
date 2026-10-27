#pragma once
#include "gui_widget_list.hpp"
namespace nfs::gui::widgets::family {
struct Tree {
    GeneralWidget base;
    TreeNode root;
    Dword style;
    int indent, row_height, anchor, cursor, expand_icon_size;
};
static_assert(sizeof(Tree) == 356 && offsetof(Tree, style) == 332);
inline Tree *tree(Widget *widget) {
    return reinterpret_cast<Tree *>(widget);
}
inline TreeNode *node_parent(TreeNode *node) {
    return reinterpret_cast<TreeNode *>(node->parent);
}
inline TreeNode *tree_child(TreeNode *node, Dword index) {
    return static_cast<TreeNode *>(events::container_value(&node->children, index));
}
TreeNode *next_visible_node(TreeNode *);
TreeNode *previous_visible_node(TreeNode *);
TreeNode *previous_node_sibling(TreeNode *);
TreeNode *last_expanded_descendant(TreeNode *);
TreeNode *tree_row(Widget *, int);
void set_node_selection(TreeNode *, Dword);
void clear_descendant_selection(TreeNode *);
int node_visible_descendants(TreeNode *);
int node_depth(TreeNode *);
void update_node_indices(TreeNode *);
int first_visible_tree_row(Widget *);
int visible_tree_rows(Widget *);
int last_visible_tree_row(Widget *, int);
void ensure_tree_row_visible(Widget *, int);
Dword select_tree_row(Widget *, int, Dword);
Dword select_tree_range(Widget *, int, int);
} // namespace nfs::gui::widgets::family
