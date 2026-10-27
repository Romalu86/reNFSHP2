#include "gui_widget_controls.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "../../math/precision/extended_float.hpp"
#include <algorithm>
#include <cmath>
namespace nfs::gui {
kernel::numeric::Extended ticks_to_seconds(int);
}
namespace nfs::gui::widgets::family {
using game::runtime::at;
struct SpinButton {
    GeneralWidget base;
    int minimum, maximum, acceleration;
    float velocity, position;
    int value;
    Dword up, down, buddy;
};
static_assert(sizeof(SpinButton) == 300);

void set_spin_button_value(Widget *widget, int value) {
    auto *spin = reinterpret_cast<SpinButton *>(widget);
    spin->velocity = 0;
    const int lower = spin->minimum > spin->maximum ? spin->maximum : spin->minimum;
    if (value < lower)
        value = lower;
    else if (value > spin->maximum)
        value = spin->maximum;
    spin->value = value;
    spin->position = float(kernel::numeric::Extended::integer(value, kernel::numeric::current_format()));
}

Dword __fastcall spin_button_load(Widget *widget, void *, Dword address) {
    auto *spin = reinterpret_cast<SpinButton *>(widget);
    void *configuration = reinterpret_cast<void *>(address);
    using namespace engine::pending;
    read_integer(configuration, "GSpinButton.mMin", &spin->minimum, 0);
    read_integer(configuration, "GSpinButton.mMax", &spin->maximum, 0);
    read_integer(configuration, "GSpinButton.mAccel", &spin->acceleration, 0);
    String buddy;
    string_construct(&buddy);
    Dword result;
    try {
        read_string(configuration, "GSpinButton.mBuddy", &buddy,
                    &lifecycle::construction::unresolved::shared_string);
        if (Widget *peer = named_sibling(widget, &buddy))
            spin->buddy = peer->id;
        spin->maximum = std::max(spin->minimum, spin->maximum);
        spin->value =
            std::min(std::max(spin->value, std::min(spin->minimum, spin->maximum)), spin->maximum);
        spin->position = float(spin->value);
        result = pending::load_general(reinterpret_cast<GeneralWidget *>(widget), nullptr,
                                       configuration);
    } catch (...) {
        string_destruct(&buddy);
        throw;
    }
    string_destruct(&buddy);
    return result;
}

Dword __fastcall spin_button_save(Widget *widget, void *, Dword address) {
    auto *spin = reinterpret_cast<SpinButton *>(widget);
    void *configuration = reinterpret_cast<void *>(address);
    using namespace engine::pending;
    write_int(configuration, "GSpinButton.mMin", spin->minimum, 0);
    write_int(configuration, "GSpinButton.mMax", spin->maximum, 0);
    write_int(configuration, "GSpinButton.mAccel", spin->acceleration, 0);
    String buddy;
    string_construct(&buddy);
    Dword result;
    try {
        if (Widget *peer = sibling_by_id(widget, spin->buddy))
            string_copy(&buddy, &peer->name);
        engine::pending::write_text(configuration, "GSpinButton.mBuddy", string_ascii(&buddy),
                                    nullptr);
        result = pending::save_general(reinterpret_cast<GeneralWidget *>(widget), nullptr,
                                       configuration);
    } catch (...) {
        string_destruct(&buddy);
        throw;
    }
    string_destruct(&buddy);
    return result;
}
static void notify_spin_change(Widget *widget, SpinButton *spin, int before, int after) {
    if (spin->buddy)
        dispatch_target(spin->buddy, 49, Dword(before), Dword(after));
    else
        events::dispatch(widget, 49, Dword(before), Dword(after));
}

void __fastcall spin_button_configure_geometry(Widget *widget, void *, Dword sender_address,
                                               Dword event, Dword state, Dword) {
    auto *spin = reinterpret_cast<SpinButton *>(widget);
    const Dword sender = reinterpret_cast<Widget *>(sender_address)->id;
    const auto step = [&](int direction) {
        const int before = spin->value;
        const int next = int(Dword(before) + Dword(direction));
        spin->velocity = 0;
        spin->value =
            std::min(std::max(next, std::min(spin->minimum, spin->maximum)), spin->maximum);
        spin->position = float(spin->value);
        if (spin->value != before)
            notify_spin_change(widget, spin, before, spin->value);
    };
    if (sender == spin->down && event == 43)
        step(-1);
    if (sender == spin->up && event == 43)
        step(1);
    if (sender == spin->down && event == 48) {
        spin->velocity = state & 1 ? float(int(Dword(0) - Dword(spin->acceleration))) : 0;
        return;
    }
    if (sender == spin->up && event == 48)
        spin->velocity = state & 1 ? float(spin->acceleration) : 0;
}

void __fastcall spin_button_tick(Widget *widget, void *, Dword ticks) {
    auto *spin = reinterpret_cast<SpinButton *>(widget);
    if (!(dispatch_target(spin->up, 47, 0, 0) & 1) &&
        !(dispatch_target(spin->down, 47, 0, 0) & 1)) {
        spin->velocity = 0;
        return;
    }
    if (spin->velocity == 0 && !std::isnan(spin->velocity))
        return;
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    const int before = spin->value;
    const float value = float(ticks_to_seconds(int(ticks)) * Extended(spin->velocity, format) +
                              Extended(spin->position, format));
    spin->position = value;
    const float maximum = float(Extended::integer(spin->maximum, format));
    const Extended minimum = Extended::integer(spin->minimum, format);
    const Extended lower =
        Extended(maximum, format) < minimum ? Extended(maximum, format) : minimum;
    Extended bounded = Extended(value, format);
    if (bounded < lower)
        bounded = lower;
    else if (Extended(maximum, format) < bounded)
        bounded = Extended(maximum, format);
    spin->position = float(bounded);
    spin->value = int(std::lrintf(spin->position));
    if (spin->value != before)
        notify_spin_change(widget, spin, before, spin->value);
    const int acceleration =
        spin->velocity > 0 ? spin->acceleration : int(Dword(0) - Dword(spin->acceleration));
    spin->velocity = float(Extended::integer(acceleration, format) * ticks_to_seconds(int(ticks)) +
                           Extended(spin->velocity, format));
}

void __fastcall spin_button_initialize(Widget *widget, void *) {
    pending::request_general_layout(reinterpret_cast<GeneralWidget *>(widget), nullptr);
    auto *spin = reinterpret_cast<SpinButton *>(widget);
    spin->up = named_child_id(widget, "<UP>");
    spin->down = named_child_id(widget, "<DOWN>");
}
} // namespace nfs::gui::widgets::family
