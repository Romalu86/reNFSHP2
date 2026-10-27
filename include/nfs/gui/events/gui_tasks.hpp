#pragma once
#include "nfs/gui/lifecycle/gui_context.hpp"
namespace nfs::gui {
using TaskCallback = void(__cdecl *)(Dword);
struct TaskObject {
    const Destroy *methods;
    TaskCallback callback;
    Dword state;
};
static_assert(sizeof(TaskObject) == 12);
void add_task(ContextState *, TaskCallback);
void remove_task(ContextState *, TaskCallback);
void register_task(TaskCallback);
void unregister_task(TaskCallback);
} // namespace nfs::gui
