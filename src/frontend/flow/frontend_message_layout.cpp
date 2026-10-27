#include "../screens/frontend_dialogs.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
namespace nfs::application::pending {

void initialize_message_box(MessageBox *owner, const gui::String *text, const gui::String *title,
                            Dword flags) {
    if (network::packets::rule_value(system(72), 17) == 6)
        flags |= 131072;
    owner->flags = flags;
    for (Dword &id : owner->buttons)
        id = 0;
    owner->button_count = int(flags & 15);
    if (owner->button_count > 3)
        owner->button_count = 4;
    LocalFrontendString name;
    gui::string_format_ascii(&name.value, "Message%db", owner->button_count);
    if (flags & 4096)
        gui::string_append_ascii(&name.value, "_lg");
    void *parent = gui::focused_widget();
    if (parent)
        parent = widget_root(static_cast<Widget *>(parent));
    if (!parent)
        parent = at<void *>(system(96), 40);
    {
        LocalFrontendString type("GMsgBox"), file("system\\GDialogs.gui");
        owner->widget =
            create_message_widget(&file.value, &name.value, &type.value, parent, text, title);
    }
    Widget *widget = owner->widget;
    if (!widget)
        return;
    gui::widgets::set_enabled(widget, 0);
    set_widget_visible(widget, 0);
    owner->base_height = widget->bounds.height;
    int found = 0;
    for (int index = 0; index < 4; ++index) {
        LocalFrontendString button_name;
        gui::string_format_ascii(&button_name.value, "<B%d>", index);
        const Dword id =
            gui::widgets::family::named_child_id(widget, gui::string_ascii(&button_name.value));
        if (id) {
            owner->buttons[found] = id;
            if (auto *button =
                    static_cast<Widget *>(gui::events::find_widget(gui::context_state, id))) {
                gui::widgets::set_enabled(button, 0);
                set_widget_visible(button, 0);
                set_widget_tab_order(button, Dword(found));
            }
            ++found;
        }
    }
    const auto label_button = [owner](int index, Dword reference, Dword result) {
        LocalFrontendString label;
        gui::string_reference(&label.value, reference);
        configure_message_button(owner, index, &label.value, result);
    };
    if ((owner->flags & 18) == 18) {
        label_button(0, 459, 2);
        label_button(1, 460, 3);
    }
    if ((owner->flags & 34) == 34) {
        label_button(0, 372, 2);
        label_button(1, 460, 3);
    }
    if ((owner->flags & 66) == 66) {
        label_button(0, 374, 2);
        label_button(1, 1334, 3);
    }
    if ((owner->flags & 257) == 257)
        label_button(0, 459, 2);
    if ((owner->flags & 513) == 513)
        label_button(0, 460, 3);
    if ((owner->flags & 129) == 129)
        label_button(0, 372, 2);
    const Dword options = owner->flags;
    if ((options & 16384) && owner->button_count >= 2)
        if (auto *button = static_cast<Widget *>(
                gui::events::find_widget(gui::context_state, owner->buttons[1])))
            set_widget_tab_order(button, 0);
    if ((options & 32768) && owner->button_count >= 3)
        if (auto *button = static_cast<Widget *>(
                gui::events::find_widget(gui::context_state, owner->buttons[2])))
            set_widget_tab_order(button, 0);
    void *title_widget = find_child(widget, "<TITLE>");
    if (title_widget && invoke<Dword>(title_widget, 2, gui::widgets::family::text_class_key))
        invoke(title_widget, 7, title);
    auto *text_widget = static_cast<Widget *>(find_child(widget, "<TEXT>"));
    if (!text_widget || !invoke<Dword>(text_widget, 2, gui::widgets::family::text_class_key))
        return;
    if (void *engine = gui::active_engine())
        if (owner->flags & 131072) {
            invoke(engine, 14, gui::widgets::inherited_font_style(text_widget));
            at<int>(text_widget, 276) = invoke<int>(engine, 17) / 2;
        }
    invoke(text_widget, 7, text);
    invoke(text_widget, 6, Dword(512));
    invoke(text_widget, 6, Dword(64));
    if (text->length > 0) {
        gui::engine::Rectangle text_bounds;
        copy_dialog_rectangle(&text_bounds, &text_widget->bounds);
        widget->bounds.height = int(Dword(owner->base_height) + Dword(text_bounds.height));
        for (int index = 0; index < owner->button_count; ++index)
            if (auto *button = static_cast<Widget *>(
                    gui::events::find_widget(gui::context_state, owner->buttons[index]))) {
                gui::engine::Point displacement;
                construct_dialog_point(&displacement, 0, text_bounds.height);
                button->bounds.left = int(Dword(button->bounds.left) + Dword(displacement.x));
                button->bounds.top = int(Dword(button->bounds.top) + Dword(displacement.y));
                displacement.methods = gui::base_methods;
            }
        text_bounds.methods = gui::base_methods;
    }
}
} // namespace nfs::application::pending
