#include "nfs/gui/events/gui_events.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::gui {

Dword container_find_index(Container *container, Compare compare, const void *key) {
    if (!compare || !container->count)
        return Dword(-1);
    const Dword count = container->count;
    if (void *first = events::container_value(container, 0))
        if (compare(first, key) > 0)
            return Dword(-1);
    if (void *last = events::container_value(container, count - 1))
        if (compare(last, key) < 0)
            return Dword(-1);
    Dword lower = 0, upper = count, previous = Dword(-1);
    for (;;) {
        const Dword middle = int(lower + upper) / 2;
        if (middle == previous || middle >= count)
            return Dword(-1);
        void *value = events::container_value(container, middle);
        if (!value)
            return Dword(-1);
        const int order = compare(value, key);
        if (!order)
            return middle;
        previous = middle;
        if (order > 0)
            upper = middle;
        else
            lower = middle;
    }
}
// Failed allocation leaves the deleted slot in the old table.
Dword container_remove_at(Container *container, Dword index) {
    const Dword count = container->count;
    if (index >= count)
        return 3;
    Element **entries = container->elements;
    if (entries[index]) {
        destroy(reinterpret_cast<Object *>(entries[index]));
        entries[index] = nullptr;
    }
    const Dword remaining = count - 1;
    if (!remaining) {
        kernel::memory_free(entries);
        container->elements = nullptr;
        container->count = 0;
        return 0;
    }
    auto **replacement = static_cast<Element **>(
        kernel::memory_allocate(nullptr, remaining * 4, container->flags | pool));
    if (!replacement)
        return 1;
    if (int(index * 4) > 0)
        kernel::memory_copy(replacement, entries, index * 4);
    const int tail = int((count - index) * 4 - 4);
    if (tail > 0)
        kernel::memory_copy(replacement + index, entries + index + 1, tail);
    kernel::memory_free(entries);
    container->count = remaining;
    container->elements = replacement;
    return 0;
}
} // namespace nfs::gui
