#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "../screens/frontend_dialogs.hpp"
#include "nfs/audio/music/music_clients.hpp"
namespace nfs::application::pending {

static void *__fastcall delete_message_box(MessageBox *owner, void *, unsigned flags) {
    destroy_message_box(owner);
    if (flags & 1)
        gui::release(owner);
    return owner;
}
const gui::Destroy message_box_methods[1] = {reinterpret_cast<gui::Destroy>(delete_message_box)};

MessageBox *construct_message_box(MessageBox *owner, const gui::String *text,
                                  const gui::String *title) {
    owner->methods = gui::base_methods;
    owner->widget = nullptr;
    owner->button_count = 0;
    owner->base_height = 0;
    owner->flags = 257;
    owner->methods = message_box_methods;
    try {
        initialize_message_box(owner, text, title, 257);
        return owner;
    } catch (...) {
        owner->methods = gui::base_methods;
        throw;
    }
}

MessageBox *construct_message_box(MessageBox *owner, const gui::String *text) {
    owner->methods = gui::base_methods;
    owner->widget = nullptr;
    owner->button_count = 0;
    owner->base_height = 0;
    owner->flags = 257;
    owner->methods = message_box_methods;
    try {
        initialize_message_box(owner, text,
                               &lifecycle::construction::unresolved::shared_string, 257);
        return owner;
    } catch (...) {
        owner->methods = gui::base_methods;
        throw;
    }
}

MessageBox *construct_message_box(MessageBox *owner, const gui::String *text, Dword flags) {
    owner->methods = gui::base_methods;
    owner->widget = nullptr;
    owner->button_count = 0;
    owner->base_height = 0;
    owner->flags = flags;
    owner->methods = message_box_methods;
    try {
        initialize_message_box(owner, text,
                               &lifecycle::construction::unresolved::shared_string, flags);
        return owner;
    } catch (...) {
        owner->methods = gui::base_methods;
        throw;
    }
}

MessageBox *construct_message_box(MessageBox *owner, const gui::String *text, const gui::String *title,
                           Dword flags) {
    owner->methods = gui::base_methods;
    owner->widget = nullptr;
    owner->button_count = 0;
    owner->base_height = 0;
    owner->flags = flags;
    owner->methods = message_box_methods;
    try {
        initialize_message_box(owner, text, title, flags);
        return owner;
    } catch (...) {
        owner->methods = gui::base_methods;
        throw;
    }
}

void destroy_message_box(MessageBox *owner) {
    owner->methods = message_box_methods;
    kernel::lifecycle::Cleanup base_cleanup([=] { owner->methods = gui::base_methods; });
    if (owner->widget) {
        destroy_screen(owner->widget);
        owner->widget = nullptr;
    }
}

void clear_message_widget(MessageBox *owner) {
    if (owner->widget) {
        destroy_screen(owner->widget);
        owner->widget = nullptr;
    }
}

Dword configure_message_button_callback(MessageBox *owner, int index, const gui::String *label,
                                       gui::widgets::Callback callback, Dword result) {
    if (index >= owner->button_count)
        return 0;
    auto *button =
        static_cast<Widget *>(gui::events::find_widget(gui::context_state, owner->buttons[index]));
    if (!button)
        return 0;
    invoke(button, 7, label);
    button->value_84 = result;
    set_widget_callback(button, 43, callback);
    gui::widgets::set_enabled(button, 1);
    set_widget_visible(button, 1);
    return 1;
}
// Signed index check is retained from CMP/JL.
Dword configure_message_button(MessageBox *owner, int index, const gui::String *label, Dword result) {
    if (index >= owner->button_count)
        return 0;
    auto *button =
        static_cast<Widget *>(gui::events::find_widget(gui::context_state, owner->buttons[index]));
    if (!button)
        return 0;
    invoke(button, 7, label);
    button->value_84 = result;
    gui::widgets::set_enabled(button, 1);
    set_widget_visible(button, 1);
    set_widget_callback(button, 43, accept_message_button);
    return 1;
}
// Callback has four cdecl arguments; only sender is consumed.
Dword __cdecl accept_message_button(void *sender, Dword, Dword, Dword) {
    auto *button = static_cast<Widget *>(sender);
    if (button->parent && button->value_84)
        gui::events::dispatch(button->parent, 24, button->value_84, button->id);
    return 0;
}

int run_message_box(MessageBox *owner, gui::ModalCallback callback, Dword context) {
    Widget *widget = owner->widget;
    if (!widget)
        return 7;
    widget->value_84 = context;
    auto *previous = static_cast<Widget *>(gui::focused_widget());
    const Dword flags = owner->flags;
    if ((flags & 65536) && previous && !gui::widgets::has_ancestor(widget, previous) &&
        invoke<Dword>(previous, 2, gui::widgets::family::dialog_class_key))
        set_widget_visible(previous, 0);
    if (flags & 8192)
        game::audio::play_frontend_event(66, 63);
    const int result = gui::run_modal(widget, callback);
    gui::widgets::set_enabled(widget, 0);
    set_widget_visible(widget, 0);
    if ((flags & 65536) && previous && !gui::widgets::has_ancestor(widget, previous) &&
        invoke<Dword>(previous, 2, gui::widgets::family::dialog_class_key))
        set_widget_visible(previous, 1);
    return result;
}
} // namespace nfs::application::pending
