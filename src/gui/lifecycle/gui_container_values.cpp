#include "nfs/gui/lifecycle/gui.hpp"
namespace nfs::gui {
extern const Destroy element_methods[];
// Allocation failure clears the slot and also returns zero.
Dword container_set_at(Container *container, void *value, Dword index, Dispose dispose) {
    if (container->count <= index)
        return 3;
    Element *&slot = container->elements[index];
    if (slot) {
        destroy(reinterpret_cast<Object *>(slot));
        slot = nullptr;
    }
    auto *element =
        static_cast<Element *>(allocate_flags(sizeof(Element), container->flags, nullptr));
    slot = element;
    if (element) {
        element->methods = element_methods;
        element->value = value;
        element->dispose = dispose;
    }
    return 0;
}

Dword container_move(Container *container, Dword first, Dword second) {
    if (first == second)
        return 0;
    const Dword count = container->count;
    if (count <= first || count <= second)
        return 6;
    Element **input = container->elements;
    if (!input)
        return 2;
    auto **output = static_cast<Element **>(
        kernel::memory_allocate(nullptr, count * 4, container->flags | pool));
    if (!output)
        return 1;
    Element *selected = input[first];
    Dword write = 0, read = 0;
    while (write < second) {
        if (*input == selected) {
            ++read;
            ++input;
        }
        output[write++] = *input++;
        ++read;
    }
    output[write++] = selected;
    if (write < container->count) {
        input = container->elements + read;
        do {
            if (*input == selected)
                ++input;
            output[write++] = *input++;
        } while (write < container->count);
    }
    kernel::memory_free(container->elements);
    container->elements = output;
    return 0;
}
} // namespace nfs::gui
