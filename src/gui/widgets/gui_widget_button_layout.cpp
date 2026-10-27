#include "gui_widget_controls.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/application/application.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;

void __fastcall button_initialize(Widget *widget, void *) {
    auto *button = reinterpret_cast<Button *>(widget);
    auto *render = active_engine();
    if (!(button->style & 1) || !render) {
        button->up = named_child_id(widget, "<UP>");
        button->down = named_child_id(widget, "<DOWN>");
        button->focus = named_child_id(widget, "<FOCUS>");
        button->focus_down = named_child_id(widget, "<FOCUSDOWN>");
        button->disabled = named_child_id(widget, "<DISABLE>");
        button->disabled_down = named_child_id(widget, "<DISABLEDOWN>");
        button->text = named_child_id(widget, "<TEXT>");
        button->icon = named_child_id(widget, "<ICON>");
    } else {
        widget->layout_flags &= ~Dword(64);
        set_widget_resource(widget, &lifecycle::construction::unresolved::shared_string);
        invoke(render, 21, inherited_palette(widget));
        auto *up = static_cast<Widget *>(create_image_box(widget));
        up->flags = 35;
        up->layout_flags = 128;
        set_z_order(up, 5);
        at<Dword>(up, 268) = 8;
        up->colour = 4287664272u;
        initialize_tree(up);
        button->up = up->id;
        auto *disabled = static_cast<Widget *>(create_image_box(widget));
        disabled->flags = 35;
        disabled->layout_flags = 128;
        at<Dword>(disabled, 268) = 8;
        set_z_order(disabled, 4);
        disabled->colour = 4287664272u;
        initialize_tree(disabled);
        button->disabled = disabled->id;
        auto *focus = static_cast<Widget *>(create_image_box(widget));
        focus->flags = 35;
        focus->layout_flags = 128;
        set_z_order(focus, 3);
        at<Dword>(focus, 268) = 8;
        focus->colour = 4288716960u;
        initialize_tree(focus);
        button->focus = focus->id;
        auto *down = static_cast<Widget *>(create_image_box(widget));
        down->flags = 35;
        down->layout_flags = 128;
        set_z_order(down, 2);
        at<Dword>(down, 268) = 4;
        down->colour = 4286545791u;
        initialize_tree(down);
        button->down = down->id;
        auto *text = static_cast<Widget *>(create_text(widget));
        text->flags = 1;
        set_z_order(text, 0);
        at<Dword>(text, 264) = 132;
        Rectangle bounds;
        client_bounds(widget, &bounds);
        text->bounds.top = bounds.top;
        text->bounds.left = bounds.left;
        text->bounds.width = bounds.width;
        text->bounds.height = bounds.height;
        engine::destroy_rectangle(&bounds);
        initialize_tree(text);
        button->text = text->id;
    }
    if (Widget *text = find_child(widget, button->text))
        invoke(text, 7, invoke<const String *>(widget, 8));
    button->last_state = Dword(-1);
    pending::request_general_layout(reinterpret_cast<GeneralWidget *>(widget), nullptr);
}
 
void __fastcall toggle_button_initialize(Widget *widget, void *) {
    button_initialize(widget, nullptr);
}

void __fastcall toggle_button_draw(Widget *widget, void *, Dword render_address) {
    auto *tag = static_cast<Widget *>(application::pending::find_child(widget, "<TAG>"));
    if (!tag)
        return;
    auto *items = reinterpret_cast<ListItems *>(game::runtime::bytes(widget) + 312);
    const int selected = selected_list_row(items);
    auto *item = items->grid.rows.count && selected >= 0
                     ? invoke<ListItem *>(items, 9, selected, Dword(0))
                     : nullptr;
    if (!item) {
        application::pending::set_widget_visible(tag, 0);
        return;
    }
    application::pending::set_widget_visible(tag, 1);
    auto *render = reinterpret_cast<engine::Engine *>(render_address);
    invoke(render, 21, inherited_palette(widget));
    Dword colour;
    tag->colour = *invoke<Dword *>(render, 23, &colour,
                                  Dword(widget->flags & 1 ? 2 : active_widget_id == widget->id));
    invoke(tag, 7, &item->text);
}

void __fastcall advance_toggle_button(Widget *widget, void *) {
    auto *items = reinterpret_cast<ListItems *>(game::runtime::bytes(widget) + 312);
    if (!items->grid.rows.count)
        return;
    int row = next_list_row(items, selected_list_row(items));
    if (row == -1)
        row = next_list_row(items, row);
    if (invoke<Dword>(items, 10, row, Dword(1)))
        events::dispatch(widget, 68, 0, 0);
}

Dword __fastcall toggle_button_key_down(Widget *widget, void *, Dword key, Dword modifiers) {
    auto *items = reinterpret_cast<ListItems *>(game::runtime::bytes(widget) + 312);
    if (key == 203 || key == 205) {
        if (!items->grid.rows.count)
            return 1;
        const int selected = selected_list_row(items);
        int row = key == 203 ? previous_list_row(items, selected) : next_list_row(items, selected);
        if (row < 0)
            row = key == 203 ? previous_list_row(items, int(items->grid.rows.count))
                              : next_list_row(items, -1);
        invoke(items, 10, row, Dword(1));
        events::dispatch(widget, 70, 0, 0);
        return 1;
    }
    return button_key_down(widget, nullptr, key, modifiers);
}

Dword __fastcall toggle_button_button_down(Widget *widget, void *, Dword button, Dword modifiers) {
    auto *items = reinterpret_cast<ListItems *>(game::runtime::bytes(widget) + 312);
    if (button == 3 || button == 4) {
        if (!items->grid.rows.count)
            return 1;
        const int selected = selected_list_row(items);
        int row = button == 3 ? next_list_row(items, selected) : previous_list_row(items, selected);
        if (row < 0)
            row = button == 3 ? next_list_row(items, -1)
                              : previous_list_row(items, int(items->grid.rows.count));
        invoke(items, 10, row, Dword(1));
        events::dispatch(widget, 70, 0, 0);
        return 1;
    }
    return button_button_down(widget, nullptr, button, modifiers);
}
} // namespace nfs::gui::widgets::family
