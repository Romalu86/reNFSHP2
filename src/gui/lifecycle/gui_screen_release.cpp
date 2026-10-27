#include "nfs/core/application/application.hpp"
#include "nfs/gui/events/gui_events.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui {
// Removes an element without disposing its contained widget.
Dword container_detach(Container *container, void *value) {
    if (!value)
        return 2;
    const Dword count = container->count;
    Dword remaining = count, removed_index = 0;
    for (Dword index = 0; index < count; ++index)
        if (auto *element = container->elements[index]; element && element->value == value) {
            element->dispose = nullptr;
            destroy(reinterpret_cast<Object *>(element));
            container->elements[index] = nullptr;
            remaining = count - 1;
            removed_index = index;
            break;
        }
    if (remaining == count)
        return 6;
    if (!remaining) {
        kernel::memory_free(container->elements);
        container->elements = nullptr;
        container->count = 0;
        return 0;
    }
    auto **replacement = static_cast<Element **>(
        kernel::memory_allocate(nullptr, remaining * 4, pool | container->flags));
    if (!replacement)
        return 1;
    if (static_cast<int>(removed_index * 4) > 0)
        kernel::memory_copy(replacement, container->elements, removed_index * 4);
    const int tail = (count - removed_index) * 4 - 4;
    if (tail > 0)
        kernel::memory_copy(replacement + removed_index, container->elements + removed_index + 1,
                            tail);
    kernel::memory_free(container->elements);
    container->elements = replacement;
    container->count = remaining;
    return 0;
}
} // namespace nfs::gui
namespace nfs::application::pending {

void destroy_screen(void *screen) {
    using namespace game::runtime;
    if (!screen)
        return;
    gui::events::propagate(screen, 20, 0, 0, 1);
    for (unsigned offset : {24u, 40u, 8u})
        gui::container_clear(reinterpret_cast<gui::Container *>(bytes(screen) + offset));
    if (void *parent = at<void *>(screen, 56)) {
        gui::container_remove(reinterpret_cast<gui::Container *>(bytes(parent) + 24), screen);
        parent = at<void *>(screen, 56);
        gui::container_remove(reinterpret_cast<gui::Container *>(bytes(parent) + 40), screen);
        parent = at<void *>(screen, 56);
        gui::container_detach(reinterpret_cast<gui::Container *>(bytes(parent) + 8), screen);
        at<void *>(screen, 56) = nullptr;
    }
    gui::destroy(static_cast<gui::Object *>(screen));
}
} // namespace nfs::application::pending
