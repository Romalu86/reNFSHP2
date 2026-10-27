#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "nfs/network/session/network_gui.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include <cstring>
namespace nfs::network::ui {
// Preserve the defined low byte and saved upper-byte reads.
Dword console_available() {
    auto &objects = lifecycle::runtime::console_objects;
    Dword value = reinterpret_cast<Dword>(objects[2]);
    if (value) {
        value = reinterpret_cast<Dword>(objects[1]);
        if (value) {
            value = reinterpret_cast<Dword>(objects[0]);
            if (value)
                return (value & 4294967040u) | 1;
        }
    }
    return value & 4294967040u;
}

void destroy_console_text(ConsoleText *message) {
    message->methods = console_base_methods;
}

void *__fastcall delete_console_text(ConsoleText *message, void *, unsigned flags) {
    destroy_console_text(message);
    if (flags & 1)
        kernel::memory_free(message);
    return message;
}

void *__fastcall delete_console_base(ConsoleText *message, void *, unsigned flags) {
    message->methods = console_base_methods;
    if (flags & 1)
        kernel::memory_free(message);
    return message;
}
const gui::Destroy console_methods[]{reinterpret_cast<gui::Destroy>(delete_console_text)};
const gui::Destroy console_base_methods[]{reinterpret_cast<gui::Destroy>(delete_console_base)};
// The whole 256-byte text footprint is copied.
ConsoleText *copy_console_text(ConsoleText *destination, const ConsoleText *source) {
    if (destination) {
        destination->methods = console_base_methods;
        kernel::lifecycle::Cleanup base([destination] {
            destination->methods = console_base_methods;
        });
        auto *out = reinterpret_cast<Dword *>(destination->text);
        auto *in = reinterpret_cast<const Dword *>(source->text);
        for (unsigned index = 0; index < 64; ++index)
            out[index] = in[index];
        destination->methods = console_methods;
        base.dismiss();
    }
    return destination;
}
// This receives the list subobject at console +4h.
void append_console_unlocked(void *list, const ConsoleText *message) {
    auto *fields = static_cast<Dword *>(list);
    auto *sentinel = reinterpret_cast<Dword *>(fields[1]);
    auto *previous = reinterpret_cast<Dword *>(sentinel[1]);
    auto *node = static_cast<Dword *>(gui::application_allocate(268));
    node[0] = reinterpret_cast<Dword>(sentinel);
    if (!previous)
        previous = node;
    node[1] = reinterpret_cast<Dword>(previous);
    sentinel[1] = reinterpret_cast<Dword>(node);
    previous[0] = reinterpret_cast<Dword>(node);
    copy_console_text(reinterpret_cast<ConsoleText *>(node + 2), message);
    ++fields[2];
}
// The saved unwind handler releases this critical section.
void append_console(void *console, const ConsoleText *message) {
    auto *bytes = static_cast<std::uint8_t *>(console);
    auto *lock = reinterpret_cast<CRITICAL_SECTION *>(bytes + 16);
    EnterCriticalSection(lock);
    try {
        append_console_unlocked(bytes + 4, message);
    } catch (...) {
        // Resume the normal unlock without rethrowing.
    }
    LeaveCriticalSection(lock);
}
 
Dword console_print(const char *text) {
    void *console = lifecycle::runtime::console_objects[1];
    if (!console || !text || std::strlen(text) >= 255)
        return 0;
    ConsoleText message;
    message.methods = console_methods;
    kernel::lifecycle::Cleanup message_owner([&message] { destroy_console_text(&message); });
    std::strcpy(message.text, text);
    append_console(console, &message);
    MSG pending;
    if (PeekMessageA(&pending, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&pending);
        DispatchMessageA(&pending);
    }
    return 1;
}
} // namespace nfs::network::ui
