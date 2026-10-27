#include "gui_widget_family.hpp"
#include "gui_widget_shared_methods.hpp"
#include "gui_widget_family_style.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/application/application.hpp"
#include <cstdio>
namespace nfs::application::pending {
void *find_child(void *, const char *);
}
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;

void __fastcall marquee_parent_leave(Widget *widget, void *, Dword parent) {
    notify_parent_leave(widget, nullptr, parent);
}

void __fastcall tool_tip_prepare_draw(Widget *widget, void *, Dword render) {
    prepare_draw(widget, nullptr, reinterpret_cast<engine::Engine *>(render));
}

void __fastcall tree_prepare_draw(Widget *widget, void *, Dword render) {
    prepare_draw(widget, nullptr, reinterpret_cast<engine::Engine *>(render));
}

void __fastcall slider_bar_parent_enter(Widget *widget, void *, Dword parent) {
    release_capture(widget);
    notify_parent_enter(widget, nullptr, parent);
}

void __fastcall marquee_parent_enter(Widget *widget, void *, Dword parent) {
    release_capture(widget);
    notify_parent_enter(widget, nullptr, parent);
}

Dword __fastcall edit_text_pointer_release(Widget *widget, void *, Dword button, Dword modifiers) {
    release_capture(widget);
    return capture_style(widget, nullptr, button, modifiers);
}

void __fastcall edit_text_parent_leave(Widget *widget, void *, Dword parent) {
    events::dispatch(widget, 59, 0, 0);
    notify_parent_leave(widget, nullptr, parent);
}

void __fastcall list_box_parent_enter(Widget *widget, void *, Dword parent) {
    release_capture(widget);
    notify_parent_enter(widget, nullptr, parent);
}

void __fastcall tree_parent_enter(Widget *widget, void *, Dword parent) {
    release_capture(widget);
    notify_parent_enter(widget, nullptr, parent);
}

void __fastcall edit_text_cancel(Widget *widget, void *) {
    invoke(widget, 7, game::runtime::bytes(widget) + 288);
}

Dword __fastcall slider_bar_event(Widget *widget, void *, Dword event, Dword first, Dword second) {
    if (event == 90)
        invoke(widget, 47);
    return pending::general_event(reinterpret_cast<GeneralWidget *>(widget), nullptr, event, first,
                                  second);
}

Dword __fastcall image_list_pointer_press(Widget *widget, void *, Dword button, Dword modifiers) {
    if (!button && application::pending::captured_widget_id != widget->id)
        capture_widget(widget);
    return pending::update_focus_style(widget, nullptr, button, modifiers);
}

Dword __fastcall frame_pointer_idle(Widget *widget, void *, Dword first, Dword second) {
    return at<Dword>(widget, 300) ? 1 : style_result_38(widget, nullptr, first, second);
}

void __fastcall edit_text_parent_enter(Widget *widget, void *, Dword parent) {
    if (!(at<Dword>(widget, 324) & 4))
        events::dispatch(widget, 64, 0, 0);
    release_capture(widget);
    notify_parent_enter(widget, nullptr, parent);
}

Dword __fastcall slider_bar_pointer_release(Widget *widget, void *, Dword button, Dword modifiers) {
    if (!button && application::pending::captured_widget_id == widget->id) {
        release_capture(widget);
        return 1;
    }
    return capture_style(widget, nullptr, button, modifiers);
}

Dword __fastcall image_list_pointer_release(Widget *widget, void *, Dword button, Dword modifiers) {
    if (application::pending::captured_widget_id == widget->id) {
        release_capture(widget);
        return 1;
    }
    return capture_style(widget, nullptr, button, modifiers);
}

Dword __fastcall button_key_down(Widget *widget, void *, Dword key, Dword modifiers) {
    if (key == 28 && (!std::uint8_t(modifiers) || (at<Dword>(widget, 296) & 8))) {
        events::dispatch(widget, 43, 28, modifiers);
        return 1;
    }
    return pending::general_key(reinterpret_cast<GeneralWidget *>(widget), nullptr, key, modifiers);
}

Dword __fastcall marquee_pointer_release(Widget *widget, void *, Dword button, Dword modifiers) {
    if (!button && application::pending::captured_widget_id == widget->id) {
        release_capture(widget);
        events::post_widget(widget, 40, 0, 0);
        return 1;
    }
    return capture_style(widget, nullptr, button, modifiers);
}

Dword __fastcall image_list_event(Widget *widget, void *, Dword event, Dword first, Dword second) {
    if (event == 56) {
        invoke(widget, 48);
        return 1;
    }
    if (event == 68) {
        invoke(widget, 47);
        return 1;
    }
    return pending::general_event(reinterpret_cast<GeneralWidget *>(widget), nullptr, event, first,
                                  second);
}

Dword __fastcall tree_event(Widget *widget, void *, Dword event, Dword first, Dword second) {
    if (event == 72) {
        invoke(widget, 48, first, second);
        return 1;
    }
    if (event == 73) {
        invoke(widget, 49);
        return 1;
    }
    return pending::general_event(reinterpret_cast<GeneralWidget *>(widget), nullptr, event, first,
                                  second);
}

Dword __fastcall button_pointer_release(Widget *widget, void *, Dword button, Dword modifiers) {
    if (application::pending::captured_widget_id == widget->id && !button) {
        release_capture(widget);
        if (invoke<Dword>(widget, 9) && !(at<Dword>(widget, 296) & 72))
            events::dispatch(widget, 43, 0, modifiers);
        return 1;
    }
    return capture_style(widget, nullptr, button, modifiers);
}

Dword __fastcall tree_load(Widget *widget, void *, Dword address) {
    void *configuration = reinterpret_cast<void *>(address);
    using namespace engine::pending;
    read_integer(configuration, "GTree.mIndent", &at<int>(widget, 336), 0);
    read_integer(configuration, "GTree.mRowHeight", &at<int>(widget, 340), 0);
    read_integer(configuration, "GTree.mExpandIconSize", &at<int>(widget, 352), 0);
    read_flags(configuration, "GTree.mTreeStyle", &at<Dword>(widget, 332), trs_lines_flags, 0);
    if (at<Dword>(widget, 332) & 8)
        at<int>(widget, 352) = int(at<Dword>(widget, 340) << 1) / 3;
    return pending::load_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

Dword __fastcall button_load(Widget *widget, void *, Dword address) {
    void *configuration = reinterpret_cast<void *>(address);
    using namespace engine::pending;
    read_flags(configuration, "GButton.mButtonStyle", &at<Dword>(widget, 296), bs_greybutton_flags,
               0);
    read_integer(configuration, "GButton.mRadioGroup", &at<int>(widget, 304), 0);
    if (at<Dword>(widget, 296) & 16)
        at<Dword>(widget, 300) = 1;
    return pending::load_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}
static Dword list_child(Widget *widget, Dword mask_offset, Dword index, const char *format) {
    if (!(at<Dword>(widget, mask_offset) & (Dword(1) << (index & 31))))
        return 0;
    char name[256];
    std::sprintf(name, format, int(index));
    return reinterpret_cast<Dword>(application::pending::find_child(widget, name));
}

Dword __fastcall list_box_header(Widget *widget, void *, Dword column) {
    return list_child(widget, 364, column, "<HEADER%d>");
}

Dword __fastcall list_box_column(Widget *widget, void *, Dword column) {
    return list_child(widget, 352, column, "<COLUMN%d>");
}

Dword __fastcall list_box_check_image(Widget *widget, void *, Dword column) {
    return list_child(widget, 356, column, "<CHECK%d>");
}

Dword __fastcall list_box_uncheck_image(Widget *widget, void *, Dword column) {
    return list_child(widget, 360, column, "<UNCHECK%d>");
}
} // namespace nfs::gui::widgets::family
