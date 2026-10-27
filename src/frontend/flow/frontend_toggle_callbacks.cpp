#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../profiles/ui/frontend_profile_controls.hpp"
#include <cstdlib>
namespace nfs::gui::widgets {
Dword widget_flag_matches(Widget *, Dword, Dword);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;

static Dword __cdecl adjust_lap_toggle(void *sender, Dword, Dword, Dword) {
    if (sender && invoke<Dword>(sender, 2, profiles::profile_list_class_key) &&
        at<Dword>(system(72), 152) == 1)
        reinterpret_cast<ProfileToggle *>(sender)->binding = 0;
    return 0;
}
// The callback only performs the type probe.
static Dword __cdecl probe_multiplayer_toggle(void *sender, Dword, Dword, Dword) {
    if (sender)
        invoke<Dword>(sender, 2, profiles::profile_list_class_key);
    return 0;
}

static Dword __cdecl toggle_label_event(void *sender, Dword event, Dword first, Dword second) {
    auto *widget = static_cast<Widget *>(sender);
    Widget *parent = widget->parent;
    auto *toggle = parent && invoke<Dword>(parent, 2, profiles::profile_list_class_key)
                       ? reinterpret_cast<ProfileToggle *>(parent)
                       : nullptr;
    Widget *button = parent && invoke<Dword>(parent, 2, family::button_class_key) ? parent : nullptr;
    switch (event) {
    case 18:
        if (button) {
            pending::set_widget_callback(widget, 11, toggle_label_event);
            pending::set_widget_callback(widget, 12, toggle_label_event);
        }
        pending::set_widget_callback(widget, 10, toggle_label_event);
        return 0;
    case 11:
        if (button && pending::captured_widget_id != widget->id && active_widget_id == button->id)
            capture_widget(widget);
        return 1;
    case 12:
        if (button && pending::captured_widget_id == widget->id) {
            release_capture(widget);
            if (active_widget_id == button->id &&
                (invoke<Dword>(widget, 9) || invoke<Dword>(button, 9))) {
                if (!first) {
                    gui::events::dispatch(button, 43, 0, second);
                    return 1;
                }
                if (toggle) {
                    int row = family::previous_list_row(&toggle->items,
                                                        family::selected_list_row(&toggle->items));
                    if (row < 0)
                        row = family::previous_list_row(&toggle->items,
                                                        int(toggle->items.grid.rows.count));
                    if (invoke<Dword>(&toggle->items, 10, Dword(row), Dword(1)))
                        gui::events::dispatch(toggle, 68, 0, 0);
                }
            }
        }
        return 1;
    case 10:
        if (parent && widget_flag_matches(parent, 256, 1))
            activate(parent);
        return 1;
    default:
        return 0;
    }
}
static CallbackRecord toggle_callbacks[3];

static void unregister_toggle_callbacks() {
    for (unsigned index = 3; index; --index)
        unregister_callback(toggle_callbacks + index - 1);
}

static void register_toggle_callbacks() {
    register_callback(toggle_callbacks, base_class_name, "GEVENT_BEGIN", 18, "ToggleButton_Tag",
                      toggle_label_event, 0);
    register_callback(toggle_callbacks + 1, profiles::profile_list_class_key, "GEVENT_IDLE", 21,
                      "ToggleButton_EnsureMultiPlayer", probe_multiplayer_toggle, 0);
    register_callback(toggle_callbacks + 2, profiles::profile_list_class_key, "GEVENT_BEGIN", 18,
                      "ToggleButton_Laps", adjust_lap_toggle, 0);
    std::atexit(unregister_toggle_callbacks);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_toggle_callbacks, ".CRT$XCU_NFS_00690144",
                       register_toggle_callbacks);
} // namespace
} // namespace nfs::application::frontend
