#include "gui_widget_list_sort.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstdlib>
namespace nfs::gui::widgets::family {
namespace {
using ContextCompare = int(__cdecl *)(const void *, const void *, Dword);
ContextCompare list_comparer;
Dword list_sort_column;
Compare grid_comparer, row_comparer;
Dword grid_sort_column;
} // namespace

ListItem *copy_list_item(ListItem *destination, const ListItem *source) {
    string_copy(&destination->text, &source->text);
    destination->data = source->data;
    destination->flags = source->flags;
    string_copy(&destination->tooltip, &source->tooltip);
    destination->image = source->image;
    return destination;
}

static int __cdecl compare_list_cells(const void *first, const void *second) {
    return list_comparer ? list_comparer(first, second, list_sort_column) : 0;
}

static int __cdecl compare_sort_elements(const void *first, const void *second) {
    if (!row_comparer)
        return 0;
    return row_comparer((*static_cast<Element *const *>(first))->value,
                        (*static_cast<Element *const *>(second))->value);
}

static int __cdecl compare_grid_rows(const void *first, const void *second) {
    auto *a = const_cast<Container *>(static_cast<const Container *>(first));
    auto *b = const_cast<Container *>(static_cast<const Container *>(second));
    const void *left = grid_sort_column < a->count && a->elements[grid_sort_column]
                           ? a->elements[grid_sort_column]->value
                           : nullptr;
    const void *right = grid_sort_column < b->count && b->elements[grid_sort_column]
                            ? b->elements[grid_sort_column]->value
                            : nullptr;
    return grid_comparer ? grid_comparer(left, right) : 0;
}

static Dword sort_item_grid(ItemGrid *grid, Dword column, Compare compare) {
    grid_sort_column = column;
    grid_comparer = compare;
    auto *rows = &grid->rows;
    if (rows->count > 1) {
        const Dword bytes = rows->count * 4;
        auto **elements = static_cast<Element **>(
            kernel::memory_allocate(nullptr, int(bytes), rows->flags | pool));
        if (elements) {
            if (int(bytes) > 0)
                kernel::memory_copy(elements, rows->elements, bytes);
            row_comparer = compare_grid_rows;
            std::qsort(elements, rows->count, 4, compare_sort_elements);
            row_comparer = nullptr;
            kernel::memory_free(rows->elements);
            rows->elements = elements;
        }
    }
    return 0;
}

void sort_list_items(ListItems *items, Dword column, ContextCompare compare) {
    if (compare) {
        list_comparer = compare;
        list_sort_column = column;
        sort_item_grid(&items->grid, column, compare_list_cells);
    }
}
} // namespace nfs::gui::widgets::family
