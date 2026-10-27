#include "gui_widget_resources.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <exception>
namespace nfs::gui::engine {
Point *__fastcall delete_point(Point *, void *, unsigned);
}
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;
Dword tooltip_owner{};
static Dword node_serial{};
const Destroy vector_methods[] = {reinterpret_cast<Destroy>(engine::delete_point)};

ImageSequence *construct_image_sequence(ImageSequence *sequence) {
    construct_image_resource(&sequence->image);
    sequence->methods = image_sequence_methods;
    sequence->texture = 0;
    sequence->frame = -1;
    sequence->frames = 0;
    return sequence;
}

void destroy_image_sequence(ImageSequence *sequence) {
    sequence->methods = image_sequence_methods;
    Object *engine = engine_object;
    const Dword texture = sequence->texture;
    if (texture && engine)
        invoke(engine, 34, texture);
    if (engine && invoke<Dword>(engine, 39, texture, sequence->image.image))
        clear_image_resource(&sequence->image);
    sequence->texture = 0;
    sequence->frame = -1;
    sequence->frames = 0;
    destroy_image_resource(&sequence->image);
    sequence->methods = reinterpret_cast<const Method *>(gui::base_methods);
}

static ImageSequence *__fastcall delete_image_sequence(ImageSequence *sequence, void *,
                                                       Dword flags) {
    destroy_image_sequence(sequence);
    if (flags & 1)
        release(sequence);
    return sequence;
}
const Method image_sequence_methods[] = {reinterpret_cast<Method>(delete_image_sequence)};

ItemGrid *construct_item_grid(ItemGrid *grid) {
    container_construct(&grid->rows);
    container_construct(&grid->columns);
    grid->methods = item_grid_methods;
    grid->options = 0;
    return grid;
}

static Dword clear_grid_rows(ItemGrid *grid) {
    return container_clear(&grid->rows);
}

void destroy_item_grid(ItemGrid *grid) {
    grid->methods = item_grid_methods;
    int cleanup_state = 2;
    try {
        container_clear(&grid->rows);
        cleanup_state = 3;
        container_destruct(&grid->columns);
        cleanup_state = 4;
        container_destruct(&grid->rows);
    } catch (...) {
        try {
            if (cleanup_state == 2)
                container_destruct(&grid->columns);
            else if (cleanup_state == 3)
                grid->columns.methods = gui::base_methods;
            if (cleanup_state == 2 || cleanup_state == 3)
                container_destruct(&grid->rows);
            else
                grid->rows.methods = gui::base_methods;
            grid->methods = reinterpret_cast<const Method *>(gui::base_methods);
        } catch (...) {
            std::terminate();
        }
        throw;
    }
    grid->methods = reinterpret_cast<const Method *>(gui::base_methods);
}

static ItemGrid *__fastcall delete_item_grid(ItemGrid *grid, void *, Dword flags) {
    destroy_item_grid(grid);
    if (flags & 1)
        release(grid);
    return grid;
}
const Method item_grid_methods[] = {reinterpret_cast<Method>(delete_item_grid)};

ListItems *construct_list_items(ListItems *grid) {
    construct_item_grid(&grid->grid);
    grid->methods = list_items_methods;
    return grid;
}

void __fastcall clear_list_selection(ListItems *grid, void *) {
    const int count = int(grid->grid.rows.count);
    for (int index = 0; index < count; ++index)
        if (void *item = invoke<void *>(grid, 9, index, 0))
            at<Dword>(item, 28) &= ~Dword(1);
}

void destroy_list_items(ListItems *grid) {
    grid->methods = list_items_methods;
    try {
        clear_list_selection(grid, nullptr);
        clear_grid_rows(&grid->grid);
    } catch (...) {
        try {
            destroy_item_grid(&grid->grid);
        } catch (...) {
            std::terminate();
        }
        throw;
    }
    destroy_item_grid(&grid->grid);
}
// String value argument is spelled as a source reference plus its copy.
TreeNode *construct_tree_node(TreeNode *node, const String *text, Dword value) {
    String copy;
    string_construct_copy(&copy, text);
    string_construct(&node->text);
    container_construct(&node->children);
    node->methods = tree_node_methods;
    node->flags = 0;
    node->parent = 0;
    node->data = value;
    try {
        string_copy(&node->text, &copy);
        node_serial += 4;
        node->id = node_serial;
        node->visible_descendants = node->visible_index = node->depth = 0;
        container_append(&tree_nodes, node, nullptr);
    } catch (...) {
        try {
            container_destruct(&node->children);
            string_destruct(&node->text);
            node->methods = reinterpret_cast<const Method *>(gui::base_methods);
            string_destruct(&copy);
        } catch (...) {
            std::terminate();
        }
        throw;
    }
    string_destruct(&copy);
    return node;
}

void destroy_tree_node(TreeNode *node) {
    node->methods = tree_node_methods;
    bool clearing_children = false;
    try {
        container_detach(&tree_nodes, node);
        clearing_children = true;
        container_destruct(&node->children);
        string_destruct(&node->text);
    } catch (...) {
        try {
            if (clearing_children)
                node->children.methods = gui::base_methods;
            else
                container_destruct(&node->children);
            string_destruct(&node->text);
            node->methods = reinterpret_cast<const Method *>(gui::base_methods);
        } catch (...) {
            std::terminate();
        }
        throw;
    }
    node->methods = reinterpret_cast<const Method *>(gui::base_methods);
}

static TreeNode *__fastcall delete_tree_node(TreeNode *node, void *, Dword flags) {
    destroy_tree_node(node);
    if (flags & 1)
        release(node);
    return node;
}
const Method tree_node_methods[] = {reinterpret_cast<Method>(delete_tree_node)};
} // namespace nfs::gui::widgets::family
