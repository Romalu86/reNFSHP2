#pragma once
#include "gui_widget_family.hpp"
#include "../lifecycle/gui_registry_owners.hpp"
namespace nfs::gui {
Dword create_widget_timer(Dword, float);
void remove_widget_timer(Dword);
void update_widget_timer(Dword, float);
} // namespace nfs::gui
namespace nfs::gui::widgets::family {
struct ImageSequence {
    const Method *methods;
    Dword texture;
    int frame;
    Dword frames;
    ImageResource image;
};
struct ItemGrid {
    const Method *methods;
    Container rows, columns;
    Dword options;
};
struct ListItems {
    const Method *methods;
    ItemGrid grid;
};
struct TreeNode {
    const Method *methods;
    Dword flags, id, parent;
    String text;
    Dword data;
    Container children;
    Dword visible_descendants, visible_index, depth;
};
static_assert(sizeof(ImageSequence) == 56 && sizeof(ItemGrid) == 40 &&
              sizeof(ListItems) == 44 && sizeof(TreeNode) == 68);
static_assert(offsetof(TreeNode, flags) == 4 && offsetof(TreeNode, parent) == 12 &&
              offsetof(TreeNode, visible_index) == 60 && offsetof(TreeNode, depth) == 64);
extern const Method image_widget_methods[47], image_sequence_methods[1], item_grid_methods[1],
    list_items_methods[14], toggle_items_methods[14], list_box_items_methods[14],
    tree_node_methods[1];
extern const Destroy vector_methods[1];
extern Dword tooltip_owner;
ImageSequence *construct_image_sequence(ImageSequence *);
void destroy_image_sequence(ImageSequence *);
ItemGrid *construct_item_grid(ItemGrid *);
void destroy_item_grid(ItemGrid *);
ListItems *construct_list_items(ListItems *);
void destroy_list_items(ListItems *);
TreeNode *construct_tree_node(TreeNode *, const String *, Dword);
void destroy_tree_node(TreeNode *);
extern const char *edit_characters;
} // namespace nfs::gui::widgets::family
