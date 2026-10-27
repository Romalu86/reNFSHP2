#include "nfs/gui/lifecycle/gui_modal.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui {
using game::runtime::at;
using game::runtime::invoke;

int run_modal(void *widget, ModalCallback callback) {
    if (!invoke<Dword>(widget, 44 / 4))
        return 7;
    at<Dword>(widget, 60) |= 128;
    if (callback)
        callback(widget);
    int result = at<int>(widget, 220);
    for (;;) {
        if (result) {
            invoke(widget, 48 / 4);
            return at<int>(widget, 220);
        }
        if (!context_state)
            events::dispatch(widget, 24, 7, at<Dword>(widget, 4));
        else {
            pump_context(context_state);
            if (poll_context(widget))
                events::dispatch(widget, 24, 7, at<Dword>(widget, 4));
            else {
                void *root = widget;
                for (void *parent = at<void *>(widget, 56); parent;
                     parent = at<void *>(parent, 56))
                    root = parent;
                if (tick_root(root))
                    events::dispatch(widget, 24, 7, at<Dword>(widget, 4));
            }
        }
        result = at<int>(widget, 220);
    }
}

void *focused_widget() {
    return context_state ? events::focused(context_state) : nullptr;
}
} // namespace nfs::gui
