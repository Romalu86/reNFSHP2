#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/gui/events/gui_events.hpp"
#include "nfs/gui/lifecycle/gui_modal.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
#include "vehicle_audio_progress_math.hpp"
#include <cstring>
namespace nfs::gui {
// Calls / have empty RET bodies in the saved artifact.
Dword tick_root(void *root) {
    if (!root)
        return 2;
    auto *engine = active_engine();
    if (!engine)
        return 13;
    game::runtime::invoke<void>(engine, 63, root);
    return 0;
}
} // namespace nfs::gui
namespace nfs::game::audio::vehicle {

const char *pending::loading_widget_type() {
    return "QuickRaceLoadingScreen";
}

Progress *construct_progress(Progress *progress, int total) {
    progress->total = total;
    progress->completed = 0;
    progress->next = nullptr;
    progress->previous = nullptr;
    if (total < 0)
        progress->total = 0;
    auto *tail = progress_head;
    if (!tail) {
        progress_head = progress;
        return progress;
    }
    while (tail->next)
        tail = tail->next;
    tail->next = progress;
    progress->previous = tail;
    return progress;
}

kernel::numeric::Extended progress_weight(const Progress *progress) {
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    unsigned char native_value[10];
    progress_math::weight_native80(progress, native_value);
    return Extended::from_native80(native_value, format);
}

kernel::numeric::Extended progress_fraction(const Progress *progress) {
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    unsigned char native_value[10];
    progress_math::fraction_native80(progress, native_value);
    return Extended::from_native80(native_value, format);
}

kernel::numeric::Extended combined_progress() {
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    unsigned char native_value[10];
    progress_math::combined_native80(native_value);
    return Extended::from_native80(native_value, format);
}

void advance_progress(Progress *progress) {
    const int total = progress->total;
    const Dword completed_bits = Dword(progress->completed) + 1u;
    int completed;
    std::memcpy(&completed, &completed_bits, sizeof completed);
    progress->completed = completed;
    if (completed > total)
        return;
    void *widget = at<void *>(runtime::system(96), 40);
    if (!widget || !runtime::invoke<Dword>(widget, 2, pending::loading_widget_type))
        return;
    if (!network::session_active() || at<std::int16_t>(runtime::system(104), 8) != 4) {
        gui::events::dispatch(widget, 108, 0, 0);
        gui::tick_root(widget);
    }
}

void destroy_progress(Progress *progress) {
    if (Progress *previous = progress->previous) {
        previous->next = progress->next;
        progress->next = nullptr;
        progress->previous = nullptr;
        return;
    }
    progress_head = nullptr;
}
} // namespace nfs::game::audio::vehicle
