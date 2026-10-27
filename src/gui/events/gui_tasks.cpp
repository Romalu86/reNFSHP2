#include "nfs/gui/events/gui_tasks.hpp"
#include "nfs/gui/events/gui_events.hpp"
namespace nfs::gui {

static void destruct_task(TaskObject *task) {
    task->methods = base_methods;
}

static void *__fastcall delete_task(TaskObject *task, void *, unsigned flags) {
    destruct_task(task);
    if (flags & 1)
        release(task);
    return task;
}
static const Destroy task_methods[] = {reinterpret_cast<Destroy>(delete_task)};

void add_task(ContextState *context, TaskCallback callback) {
    if (!callback)
        return;
    Dword count = context->first.count;
    for (Dword index = 0; index < count; ++index) {
        auto *task = static_cast<TaskObject *>(events::container_value(&context->first, index));
        if (task && task->callback == callback)
            return;
    }
    auto *task = static_cast<TaskObject *>(allocate_flags(12, 0, "GTask"));
    if (task) {
        task->methods = task_methods;
        task->callback = callback;
        task->state = 0;
    }
    container_append(&context->first, task, dispose_object);
}

void remove_task(ContextState *context, TaskCallback callback) {
    for (Dword index = 0; index < context->first.count; ++index) {
        auto *task = static_cast<TaskObject *>(events::container_value(&context->first, index));
        if (task && task->callback == callback) {
            container_remove(&context->first, task);
            return;
        }
    }
}

void register_task(TaskCallback callback) {
    if (context_state)
        add_task(context_state, callback);
}

void unregister_task(TaskCallback callback) {
    if (context_state)
        remove_task(context_state, callback);
}
} // namespace nfs::gui
