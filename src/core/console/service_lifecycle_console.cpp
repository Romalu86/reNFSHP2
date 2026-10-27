#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include "nfs/network/session/network_gui.hpp"
#include "nfs/gui/events/gui_tasks.hpp"
#include "nfs/input/window/window_input.hpp"
#include <cstring>

namespace nfs::services {
namespace {

void reset_console_base(void *object) {
    field<const gui::Destroy *>(object, 0) = network::ui::console_base_methods;
}
// This receives the list subobject at queue+4.
void destroy_console_list(void *list) {
    auto *fields = static_cast<Dword *>(list);
    auto *sentinel = reinterpret_cast<Dword *>(fields[1]);
    auto *node = reinterpret_cast<Dword *>(sentinel[0]);
    if (node != sentinel) {
        Dword count = fields[2];
        do {
            auto *previous = reinterpret_cast<Dword *>(node[1]);
            auto *next = reinterpret_cast<Dword *>(node[0]);
            auto *payload = reinterpret_cast<gui::Object *>(node + 2);
            const gui::Destroy *methods = payload->methods;
            previous[0] = reinterpret_cast<Dword>(next);
            next[1] = reinterpret_cast<Dword>(previous);
            (*methods)(payload, 0);
            gui::application_release(node);
            --count;
            node = next;
        } while (node != sentinel);
        fields[2] = count;
    }
    gui::application_release(sentinel);
    fields[1] = 0;
    fields[2] = 0;
}
void *create_queue() {
    auto *queue =
        static_cast<Dword *>(kernel::memory_allocate("ConsoleObj", 40, lifecycle::level_pool));
    if (!queue)
        return nullptr;
    kernel::lifecycle::Cleanup allocation([queue] { kernel::memory_free(queue); });
    queue[0] = reinterpret_cast<Dword>(network::ui::console_base_methods);
    kernel::lifecycle::Cleanup base([queue] { reset_console_base(queue); });
    // The initializer copies an unspecified stack byte into queue+4. No value is invented.
    auto *sentinel = static_cast<Dword *>(gui::application_allocate(268));
    sentinel[0] = sentinel[1] = reinterpret_cast<Dword>(sentinel);
    queue[2] = reinterpret_cast<Dword>(sentinel);
    queue[3] = 0;
    kernel::lifecycle::Cleanup list([queue] { destroy_console_list(queue + 1); });
    queue[0] = reinterpret_cast<Dword>(pending::console_queue_methods);
    InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION *>(queue + 4));
    list.dismiss();
    base.dismiss();
    allocation.dismiss();
    return queue;
}
// The instance is captured by the caller before task registration.
void *construct_console_window(void *console, HINSTANCE instance) {
    field<const gui::Destroy *>(console, 0) = network::ui::console_base_methods;
    kernel::lifecycle::Cleanup base([console] { reset_console_base(console); });
    field<const gui::Destroy *>(console, 0) = pending::console_window_methods;
    gui::register_task(pump_console_messages);
    if (!pending::console_window) {
        pending::console_window = CreateDialogParamA(instance, MAKEINTRESOURCEA(104), nullptr,
                                                      pending::console_dialog, 0);
        ShowWindow(pending::console_window, SW_SHOW);
        SetTimer(pending::console_window, 101, 100, nullptr);
    }
    base.dismiss();
    return console;
}
// Native iterator return is discarded by this private adapter.
void erase_queue_node(void *queue, Dword *node) {
    auto *next = reinterpret_cast<Dword *>(node[0]);
    auto *previous = reinterpret_cast<Dword *>(node[1]);
    previous[0] = reinterpret_cast<Dword>(next);
    next[1] = reinterpret_cast<Dword>(previous);
    auto *payload = reinterpret_cast<gui::Object *>(node + 2);
    (*payload->methods)(payload, 0);
    gui::application_release(node);
    --field<Dword>(queue, 12);
}
} // namespace
namespace pending {
const gui::Destroy console_queue_methods[] = {reinterpret_cast<gui::Destroy>(delete_console_queue)};
const gui::Destroy console_window_methods[] = {
    reinterpret_cast<gui::Destroy>(delete_console_window)};
} // namespace pending

void __cdecl pump_console_messages(Dword) {
    MSG message;
    if (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
}

void initialize_console(void *) {
    auto &objects = lifecycle::runtime::console_objects;
    void *console = objects[0];
    if (lifecycle::dedicated && pending::server_mode) {
        objects[2] = static_cast<Dword *>(create_queue());
        objects[1] = static_cast<Dword *>(create_queue());
        console = kernel::memory_allocate("ConsoleObj", 4, lifecycle::level_pool);
        if (console) {
            kernel::lifecycle::Cleanup allocation([console] { kernel::memory_free(console); });
            HINSTANCE instance = pending::application_instance;
            console = construct_console_window(console, instance);
            allocation.dismiss();
        }
    }
    objects[0] = static_cast<Dword *>(console);
}

void destroy_console_queue(void *object) {
    field<const gui::Destroy *>(object, 0) = pending::console_queue_methods;
    kernel::lifecycle::Cleanup base([object] { reset_console_base(object); });
    {
        kernel::lifecycle::Cleanup list([object] {
            destroy_console_list(static_cast<Byte *>(object) + 4);
        });
        DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION *>(static_cast<Byte *>(object) + 16));
        list.dismiss();
    }
    auto *sentinel = field<Dword *>(object, 8);
    auto *node = reinterpret_cast<Dword *>(sentinel[0]);
    while (node != sentinel) {
        auto *next = reinterpret_cast<Dword *>(node[0]);
        erase_queue_node(object, node);
        node = next;
    }
    gui::application_release(field<void *>(object, 8));
    field<Dword>(object, 8) = field<Dword>(object, 12) = 0;
    base.run();
}

void destroy_console_window(void *object) {
    field<const gui::Destroy *>(object, 0) = pending::console_window_methods;
    kernel::lifecycle::Cleanup base([object] { reset_console_base(object); });
    if (pending::console_window) {
        KillTimer(pending::console_window, 101);
        DestroyWindow(pending::console_window);
        pending::console_window = nullptr;
    }
    base.run();
}

void *__fastcall delete_console_queue(void *object, void *, unsigned flags) {
    destroy_console_queue(object);
    if (flags & 1)
        kernel::memory_free(object);
    return object;
}

void *__fastcall delete_console_window(void *object, void *, unsigned flags) {
    destroy_console_window(object);
    if (flags & 1)
        kernel::memory_free(object);
    return object;
}

Dword pop_console_message(void *object, void *output) {
    auto *lock = reinterpret_cast<CRITICAL_SECTION *>(static_cast<Byte *>(object) + 16);
    EnterCriticalSection(lock);
    Dword result = 0;
    try {
        if (field<Dword>(object, 12)) {
            auto *sentinel = field<Dword *>(object, 8);
            auto *first = reinterpret_cast<Dword *>(sentinel[0]);
            if (first != sentinel) {
                for (unsigned index = 0; index < 64; ++index)
                    field<Dword>(output, 4 + index * 4) = first[3 + index];
                sentinel = field<Dword *>(object, 8);
                erase_queue_node(object, reinterpret_cast<Dword *>(sentinel[0]));
                result = 1;
            }
        }
    } catch (...) {
        // Resume the normal unlock with failure still selected.
    }
    LeaveCriticalSection(lock);
    return result;
}
 
LRESULT CALLBACK console_key_hook(int code, WPARAM key, LPARAM data) {
    auto *message = reinterpret_cast<MSG *>(data);
    if (message && pending::console_window) {
        HWND input = GetDlgItem(pending::console_window, 1001);
        if (message->hwnd == input && message->message == WM_KEYDOWN &&
            ((static_cast<Dword>(message->lParam) >> 16) & 127) == 28) {
            network::ui::ConsoleText line;
            line.methods = network::ui::console_methods;
            kernel::lifecycle::Cleanup line_owner([&line] {
                network::ui::destroy_console_text(&line);
            });
            if (GetDlgItemTextA(pending::console_window, 1001, line.text, 255) && *line.text) {
                network::ui::append_console(lifecycle::runtime::console_objects[2], &line);
                SetDlgItemTextA(pending::console_window, 1001, nullptr);
            }
        }
    }
    return CallNextHookEx(pending::console_hook, code, key, data);
}
 
INT_PTR CALLBACK pending::console_dialog(HWND window, UINT message, WPARAM value, LPARAM) {
    if (message == WM_INITDIALOG) {
        console_hook = SetWindowsHookExA(WH_GETMESSAGE, console_key_hook, application_instance,
                                         GetCurrentThreadId());
        return 0;
    }
    if (message == WM_DESTROY) {
        if (console_hook) {
            UnhookWindowsHookEx(console_hook);
            console_hook = nullptr;
        }
    } else if (message == WM_CLOSE) {
        network::ui::ConsoleText line;
        line.methods = network::ui::console_methods;
        kernel::lifecycle::Cleanup line_owner([&line] {
            network::ui::destroy_console_text(&line);
        });
        std::strcpy(line.text, "/exit");
        network::ui::append_console(lifecycle::runtime::console_objects[2], &line);
    } else if (message == WM_TIMER && value == 101) {
        network::ui::ConsoleText line;
        line.methods = network::ui::console_methods;
        kernel::lifecycle::Cleanup line_owner([&line] {
            network::ui::destroy_console_text(&line);
        });
        if (pop_console_message(lifecycle::runtime::console_objects[1], &line)) {
            HWND list = GetDlgItem(window, 1003);
            if (list) {
                const LRESULT count =
                    SendMessageA(list, LB_GETCOUNT, 0, reinterpret_cast<LPARAM>(line.text));
                if (count > 200)
                    SendMessageA(list, LB_DELETESTRING, 0, 0);
                const LRESULT added =
                    SendMessageA(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(line.text));
                SendMessageA(list, LB_SETTOPINDEX, added, 0);
            }
        }
    }
    return 0;
}
} // namespace nfs::services
