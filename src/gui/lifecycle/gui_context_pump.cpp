#include "nfs/gui/lifecycle/gui_modal.hpp"
#include "nfs/gui/events/gui_tasks.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/timing/timer.hpp"
namespace nfs::gui {
using game::runtime::at;

void pump_context(ContextState *context) {
    Dword budget = events::list_count(&context->index);
    auto *message = static_cast<events::Message *>(events::pop_front(&context->index));
    while (message) {
        if (void *widget = events::find_widget(context, message->target))
            events::dispatch(widget, message->event, message->first, message->second);
        destroy(reinterpret_cast<Object *>(message));
        if (!context->value_58) {
            if (!--budget)
                break;
        } else {
            budget = events::list_count(&context->index);
            context->value_58 = 0;
        }
        message = static_cast<events::Message *>(events::pop_front(&context->index));
    }
    const Dword now = kernel::timer_count();
    if (!context->value_50)
        context->value_50 = now - context->references;
    if (static_cast<int>(context->references) <= static_cast<int>(now - context->value_50)) {
        for (Dword index = 0; index < context->third.count; ++index) {
            void *widget = events::container_value(&context->third, index);
            if (widget) {
                const Dword previous = at<Dword>(widget, 248);
                const Dword elapsed =
                    !previous || static_cast<int>(now) <= static_cast<int>(previous)
                        ? context->references
                        : now - previous;
                at<Dword>(widget, 248) = now;
                events::dispatch(widget, 21, elapsed, 0);
            }
        }
        for (Dword index = 0; index < context->first.count; ++index) {
            auto *task = static_cast<TaskObject *>(events::container_value(&context->first, index));
            const auto callback = task ? task->callback : nullptr;
            if (callback) {
                const Dword elapsed =
                    !task->state || static_cast<int>(now) <= static_cast<int>(task->state)
                        ? context->references
                        : now - task->state;
                task->state = now;
                callback(elapsed);
            }
        }
        context->value_50 = now;
    }
    kernel::cooperate(0);
    tick_registered_timers();
}
} // namespace nfs::gui
