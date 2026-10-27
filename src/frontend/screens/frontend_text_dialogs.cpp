#include "frontend_dialogs.hpp"
#include "../../gui/widgets/gui_widget_edit.hpp"
namespace nfs::gui::widgets::family {
void set_edit_character_limit(Widget *, int);
}
namespace nfs::application::pending {
namespace {
struct ConsumedString {
    gui::String *value;
    ~ConsumedString() {
        gui::string_destruct(value);
    }
};
Widget *typed_child(Widget *owner, const char *name, gui::widgets::TypeName type) {
    auto *child = static_cast<Widget *>(find_child(owner, name));
    return child && invoke<Dword>(child, 2, type) ? child : nullptr;
}
} // namespace
 
const char *__cdecl typewriter_class_key() {
    return "HP2TypeWriter";
}

Dword __cdecl accept_text_dialog_button(void *sender, Dword, Dword, Dword) {
    auto *button = static_cast<Widget *>(sender);
    if (button->parent && button->value_84)
        gui::events::dispatch(button->parent, 24, button->value_84, button->id);
    return 0;
}
// Both by-value strings are consumed.
void configure_dialog_text(Widget *dialog, gui::String text, gui::String title) {
    ConsumedString title_owner{&title}, text_owner{&text};
    if (Widget *child = typed_child(dialog, "<TITLE>", gui::widgets::family::text_class_key))
        invoke(child, 7, &title);
    if (Widget *child = typed_child(dialog, "<TEXT>", gui::widgets::family::text_class_key)) {
        invoke(child, 7, &text);
        invoke(child, 6, Dword(512));
        invoke(child, 6, Dword(64));
        if (text.length > 0) {
            const Dword height = Dword(child->bounds.height);
            dialog->bounds.height = int(Dword(dialog->bounds.height) + height);
            for (const char *name : {"<B1>", "<B2>", "<B3>", "<B4>"})
                if (auto *button = static_cast<Widget *>(find_child(dialog, name)))
                    button->bounds.top = int(Dword(button->bounds.top) + height);
        }
    }
    if (Widget *edit = typed_child(dialog, "<EDIT>", gui::widgets::family::edit_text_class_key)) {
        gui::widgets::set_enabled(edit, 0);
        set_widget_visible(edit, 0);
    }
}

void configure_text_entry_dialog(Widget *dialog, gui::String text, gui::String title) {
    ConsumedString title_owner{&title}, text_owner{&text};
    if (!dialog)
        return;
    auto *accept = static_cast<Widget *>(find_child(dialog, "<B1>"));
    auto *middle = static_cast<Widget *>(find_child(dialog, "<B2>"));
    auto *cancel = static_cast<Widget *>(find_child(dialog, "<B3>"));
    const auto configure = [](Widget *button, Dword reference, Dword result) {
        if (!button)
            return;
        gui::widgets::set_enabled(button, 1);
        set_widget_visible(button, 1);
        {
            LocalFrontendString label;
            gui::string_reference(&label.value, reference);
            invoke(button, 7, &label.value);
        }
        button->value_84 = result;
        set_widget_callback(button, 43, accept_text_dialog_button);
    };
    configure(accept, 459, 2);
    if (middle) {
        gui::widgets::set_enabled(middle, 0);
        set_widget_visible(middle, 0);
    }
    configure(cancel, 460, 3);
    {
        LocalFrontendString title_copy(&title), text_copy(&text);
        configure_dialog_text(dialog, text_copy.release(), title_copy.release());
    }
    Widget *label = typed_child(dialog, "<TEXT>", gui::widgets::family::text_class_key);
    Widget *edit = typed_child(dialog, "<EDIT>", gui::widgets::family::edit_text_class_key);
    if (label && edit) {
        gui::engine::Rectangle bounds;
        copy_dialog_rectangle(&bounds, &edit->bounds);
        bounds.top = int(Dword(label->bounds.height) + 8 + Dword(label->bounds.top));
        gui::engine::copy_rectangle_fields(&edit->bounds, &bounds);
        dialog->bounds.height = int(Dword(dialog->bounds.height) + 16 + Dword(bounds.height));
        bounds.methods = gui::base_methods;
    }
    if (edit) {
        gui::widgets::set_enabled(edit, 1);
        set_widget_visible(edit, 1);
    }
}
// Named resource falls back to Generic2b1e exactly once.
Widget *create_text_entry_dialog(Widget *parent, gui::String text, gui::String title,
                                 gui::String *initial, int limit, gui::String name, Dword timeout,
                                 Dword context) {
    ConsumedString name_owner{&name}, title_owner{&title}, text_owner{&text};
    Widget *dialog = nullptr;
    if (name.length > 0) {
        LocalFrontendString type("GMsgBox"), file("system\\GDialogs.gui");
        dialog = create_message_widget(&file.value, &name, &type.value, parent, &text, &title);
    }
    if (!dialog) {
        LocalFrontendString type("GMsgBox"), fallback("Generic2b1e"), file("system\\GDialogs.gui");
        dialog =
            create_message_widget(&file.value, &fallback.value, &type.value, parent, &text, &title);
        if (!dialog)
            return nullptr;
    }
    {
        LocalFrontendString title_copy(&title), text_copy(&text);
        configure_text_entry_dialog(dialog, text_copy.release(), title_copy.release());
    }
    Widget *edit = typed_child(dialog, "<EDIT>", gui::widgets::family::edit_text_class_key);
    if (!edit)
        edit = typed_child(dialog, "<EDIT>", typewriter_class_key);
    if (edit) {
        invoke(edit, 7, initial);
        if (limit)
            gui::widgets::family::set_edit_character_limit(edit, limit);
        edit->flags |= 4;
    }
    at<Dword>(dialog, 280) = timeout;
    dialog->value_84 = context;
    at<Dword>(dialog, 272) = 3;
    return dialog;
}

int prompt_text_entry(Widget *parent, gui::String text, gui::String title, gui::String *output,
                      const gui::String *initial, int limit, gui::String name, Dword edit_style,
                      gui::ModalCallback callback, Dword timeout, Dword context) {
    ConsumedString name_owner{&name}, title_owner{&title}, text_owner{&text};
    Widget *dialog;
    {
        LocalFrontendString name_copy(&name), title_copy(&title), text_copy(&text);
        dialog = create_text_entry_dialog(parent, text_copy.release(), title_copy.release(),
                                          const_cast<gui::String *>(initial), limit,
                                          name_copy.release(), timeout, context);
    }
    if (!dialog)
        return 7;
    Widget *edit = typed_child(dialog, "<EDIT>", gui::widgets::family::edit_text_class_key);
    if (edit) {
        if (edit_style)
            at<Dword>(edit, 320) = edit_style;
        at<Dword>(edit, 320) |= 65536;
    }
    const int result = gui::run_modal(dialog, callback);
    if (result == 2 && edit)
        gui::string_copy(output, invoke<const gui::String *>(edit, 8));
    destroy_screen(dialog);
    return result;
}
} // namespace nfs::application::pending
