#include "nfs/gui/events/gui_events.hpp"
#include "nfs/network/session/network_runtime.hpp"
namespace nfs::gui::events {
using network::field;

void *container_value(Container *container, Dword index) {
    if (index < container->count && container->elements[index])
        return container->elements[index]->value;
    return nullptr;
}

Dword call_handlers(Container *handlers, void *widget, Dword event, Dword first, Dword second) {
    if (widget) {
        const Dword count = handlers->count;
        for (Dword i = 0; i < count; ++i) {
            auto *handler = static_cast<Dword *>(container_value(handlers, i));
            if (handler && handler[1] == event && handler[2]) {
                using Callback = Dword(__cdecl *)(void *, Dword, Dword, Dword);
                return reinterpret_cast<Callback>(handler[2])(widget, event, first, second);
            }
        }
    }
    return 0;
}

void notify_handler(Container *handlers, void *widget, Dword event) {
    if (widget) {
        const Dword count = handlers->count;
        for (Dword i = 0; i < count; ++i) {
            auto *handler = static_cast<Dword *>(container_value(handlers, i));
            if (handler && handler[1] == event && handler[3]) {
                request_focus(widget, handler[3]);
                return;
            }
        }
    }
}

Dword dispatch(void *widget, Dword event, Dword first, Dword second) {
    Dword result = call_handlers(&context_state->second, widget, event, first, second);
    if (result)
        return result;
    using Handle = Dword(__thiscall *)(void *, Dword, Dword, Dword);
    const auto *methods = field<const Handle *>(widget, 0);
    if (methods[10](widget, event, first, second))
        return 0;
    auto *handlers = field<Container *>(widget, 212);
    if (handlers && !(field<std::uint8_t>(widget, 252) & 4)) {
        result = call_handlers(handlers, widget, event, first, second);
        if (result)
            return result;
    }
    result = field<const Handle *>(widget, 0)[13](widget, event, first, second);
    if (auto *current = field<Container *>(widget, 212))
        notify_handler(current, widget, event);
    if ((field<Dword>(widget, 60) & 512) && field<void *>(widget, 56) && event != 21 &&
        event != 1) {
        void *parent = field<void *>(widget, 56);
        using Bubble = void(__thiscall *)(void *, void *, Dword, Dword, Dword);
        field<const Bubble *>(parent, 0)[14](parent, widget, event, first, second);
    }
    return result;
}
// Count is captured before callbacks; each child lookup is live.
void propagate(void *widget, Dword event, Dword first, Dword second, Dword flags) {
    auto *children = reinterpret_cast<Container *>(static_cast<std::uint8_t *>(widget) + 8);
    if (flags & 4) {
        const int count = static_cast<int>(children->count);
        for (int i = 0; i < count; ++i)
            if (void *child = container_value(children, i)) {
                if (flags & 2)
                    post(context_state, field<Dword>(child, 4), event, first, second, 0);
                else
                    dispatch(child, event, first, second);
            }
        return;
    }
    const auto invoke = [&] {
        if (flags & 2)
            post(context_state, field<Dword>(widget, 4), event, first, second, 0);
        else
            dispatch(widget, event, first, second);
    };
    if (!(flags & 1))
        invoke();
    const int count = static_cast<int>(children->count);
    for (int i = 0; i < count; ++i)
        if (void *child = container_value(children, i))
            propagate(child, event, first, second, flags);
    if (flags & 1)
        invoke();
}

void post_widget(void *widget, Dword event, Dword first, Dword second) {
    post(context_state, field<Dword>(widget, 4), event, first, second, 0);
}
} // namespace nfs::gui::events
