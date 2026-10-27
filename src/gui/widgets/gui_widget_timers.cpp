#include "gui_widget_resources.hpp"
#include <cmath>
namespace nfs::gui {
extern Container timer_entries;
int compare_timer_id(const void *, const void *);
namespace {
struct WidgetTimer {
    const Destroy *methods;
    Dword widget;
    float period;
    Dword id;
    float deadline;
};
Dword timer_serial{};
extern const Destroy widget_timer_methods[];
} // namespace

static void destroy_widget_timer(WidgetTimer *timer) {
    timer->methods = base_methods;
}

static WidgetTimer *__fastcall delete_widget_timer(WidgetTimer *timer, void *, Dword flags) {
    destroy_widget_timer(timer);
    if (flags & 1)
        release(timer);
    return timer;
}
namespace {
const Destroy widget_timer_methods[] = {reinterpret_cast<Destroy>(delete_widget_timer)};
}

Dword create_widget_timer(Dword widget, float period) {
    if (!widget || period < 0)
        return 0;
    auto *timer =
        static_cast<WidgetTimer *>(allocate_flags(sizeof(WidgetTimer), 0, "GTimerElement"));
    if (timer) {
        timer->period = period;
        timer_serial += 4;
        timer->widget = widget;
        timer->methods = widget_timer_methods;
        timer->id = timer_serial;
        timer->deadline = 0;
    }
    if (container_sorted_insert(&timer_entries, compare_timer_id,
                                reinterpret_cast<void *>(timer->id), timer, dispose_object)) {
        (*timer->methods)(timer, 1);
        return 0;
    }
    return timer->id;
}

void remove_widget_timer(Dword id) {
    if (!id)
        return;
    const int index =
        int(container_find_index(&timer_entries, compare_timer_id, reinterpret_cast<void *>(id)));
    if (index >= 0)
        container_remove_at(&timer_entries, Dword(index));
}

void update_widget_timer(Dword id, float period) {
    if (!id || period < 0)
        return;
    if (auto *timer = static_cast<WidgetTimer *>(
            container_find(&timer_entries, compare_timer_id, reinterpret_cast<void *>(id))))
        timer->period = period;
}
} // namespace nfs::gui
