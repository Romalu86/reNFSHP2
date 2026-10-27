#include "gui_widget_list.hpp"
#include "gui_widget_edit.hpp"
#include "gui_widget_combo.hpp"
#include "nfs/core/application/application.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;

Widget *combo_list(Widget *widget) {
    auto *list = static_cast<Widget *>(application::pending::find_child(widget, "<LIST>"));
    return list && invoke<Dword>(list, 2, reinterpret_cast<const void *>(list_box_class_key))
               ? list
               : nullptr;
}
namespace {
bool combo_list_visible(Widget *widget) {
    Widget *list = combo_list(widget);
    return list && ((at<Dword>(widget, 336) & 2) || !(list->flags & 2));
}
} // namespace

static int find_combo_text(Widget *widget, const String *text) {
    if (Widget *list = combo_list(widget))
        for (int row = 0; row < int(list_box(list)->items.grid.rows.count); ++row)
            if (auto *item = invoke<ListItem *>(&list_box(list)->items, 9, row, Dword(0));
                item && string_equal(&item->text, text))
                return row;
    return -1;
}

Dword select_combo_row(Widget *widget, int row) {
    Widget *list = combo_list(widget);
    if (!list)
        return 0;
    auto *items = &list_box(list)->items;
    auto *item = invoke<ListItem *>(items, 9, row, Dword(0));
    if (!item || row < 0 || row >= int(items->grid.rows.count) ||
        (!(item->flags & 1) && !invoke<Dword>(items, 10, row, Dword(1))))
        return 0;
    invoke(widget, 7, &item->text);
    set_edit_anchor(widget, 0);
    set_edit_caret(widget, invoke<const String *>(widget, 8)->length, 1);
    at<int>(widget, 340) = row;
    return 1;
}

static void show_combo_list(Widget *widget, Dword visible) {
    Widget *list = combo_list(widget);
    if (!list || (at<Dword>(widget, 336) & 2))
        return;
    if (visible && (list_box(list)->style & 64) && !list_box(list)->items.grid.rows.count)
        return;
    if (at<int>(widget, 332))
        if ((list_box(list)->style & 64) && at<int>(widget, 332) >= 0)
            list_box(list)->fitted_rows = at<Dword>(widget, 332);
    if (visible) {
        list->flags = (list->flags & ~Dword(4)) | 37536;
        list_box(list)->style |= 208;
        widget->flags |= 16384;
    } else
        widget->flags &= ~Dword(16384);
    const bool was_visible = combo_list_visible(widget);
    if (was_visible != (visible != 0)) {
        if (visible)
            for (Dword index = 0; index < combo_boxes.count; ++index)
                if (auto *other =
                        static_cast<Widget *>(events::container_value(&combo_boxes, index));
                    other && combo_list_visible(other))
                    show_combo_list(other, 0);
        events::dispatch(widget, 71, visible, 0);
    }
    set_enabled(list, visible);
    application::pending::set_widget_visible(list, visible);
    if (visible) {
        widget->scroll.x = engine::default_point.x;
        widget->scroll.y = engine::default_point.y;
        const int selected = at<int>(widget, 340);
        list->scroll.x = engine::default_point.x;
        list->scroll.y = engine::default_point.y;
        const int row = selected < 0 ? next_list_row(&list_box(list)->items, -1) : selected;
        invoke<Dword>(&list_box(list)->items, 10, row, Dword(1));
        ensure_list_box_row_visible(list, row);
    }
    const Dword button = at<Dword>(widget, 328);
    if (button)
        if (auto *owner = static_cast<Widget *>(events::find_widget(context_state, button)))
            events::dispatch(owner, 45, visible, 0);
}

void __fastcall combo_box_initialize(Widget *widget, void *) {
    at<Dword>(widget, 328) = named_child_id(widget, "<BUTTON>");
    show_combo_list(widget, 0);
    pending::request_general_layout(reinterpret_cast<GeneralWidget *>(widget), nullptr);
}

void __fastcall combo_box_prepare_draw(Widget *widget, void *, Dword render_address) {
    edit_text_prepare_draw(widget, nullptr, render_address);
    Dword enabled = 0;
    if (!(widget->flags & 1))
        if (Widget *list = combo_list(widget))
            enabled = list_box(list)->items.grid.rows.count != 0;
    dispatch_target(at<Dword>(widget, 328), 25, enabled, 0);
}

void __fastcall combo_box_parent_enter(Widget *widget, void *, Dword parent_address) {
    if (parent_address && !descendant(widget, reinterpret_cast<Widget *>(parent_address)->id)) {
        show_combo_list(widget, 0);
        edit_text_parent_enter(widget, nullptr, parent_address);
        return;
    }
    notify_parent_enter(widget, nullptr, parent_address);
}

Dword __fastcall combo_box_event(Widget *widget, void *, Dword event, Dword first, Dword second) {
    switch (event) {
    case 6:
    case 8:
        if ((first == 208 || first == 200) && !combo_list_visible(widget)) {
            show_combo_list(widget, 1);
            if (!select_combo_row(widget,
                                  find_combo_text(widget, invoke<const String *>(widget, 8))))
                select_combo_row(widget, 0);
            return 1;
        }
        [[fallthrough]];
    case 3:
    case 4:
    case 5:
        if (combo_list_visible(widget))
            if (Widget *list = combo_list(widget))
                events::dispatch(list, event, first, second);
        break;
    case 11:
        if (!first && invoke<Dword>(widget, 9) && (at<Dword>(widget, 324) & 1024) &&
            !combo_list_visible(widget) && !std::uint8_t(second)) {
            show_combo_list(widget, 1);
            if (!select_combo_row(widget,
                                  find_combo_text(widget, invoke<const String *>(widget, 8))))
                select_combo_row(widget, 0);
            activate(widget);
            return 1;
        }
        break;
    case 39:
        if (combo_list_visible(widget)) {
            if (Widget *list = combo_list(widget))
                return events::dispatch(list, event, first, second);
            return 0;
        }
        break;
    case 70:
        invoke(widget, 55);
        return 1;
    case 71:
        invoke(widget, 56, first);
        return 1;
    }
    return edit_text_event(widget, nullptr, event, first, second);
}

void __fastcall validate_combo_text(Widget *widget, void *, Dword source) {
    validate_edit_text(widget, nullptr, source);
    int row;
    if (at<Dword>(widget, 336) & 1) {
        row = find_combo_text(widget, invoke<const String *>(widget, 8));
        if (select_combo_row(widget, row))
            return;
        ListItem item;
        string_construct(&item.text);
        string_construct(&item.tooltip);
        item.methods = list_item_methods;
        try {
            string_copy(&item.text, invoke<const String *>(widget, 8));
            item.data = item.flags = item.image = 0;
            if (Widget *list = combo_list(widget))
                invoke<Dword>(&list_box(list)->items, 7, &item,
                             list_box(list)->items.grid.rows.count, Dword(0));
        } catch (...) {
            string_destruct(&item.tooltip);
            string_destruct(&item.text);
            throw;
        }
        string_destruct(&item.tooltip);
        string_destruct(&item.text);
        row = find_combo_text(widget, invoke<const String *>(widget, 8));
    } else {
        if (at<Dword>(widget, 324) & 1024)
            return;
        row = find_combo_text(widget, invoke<const String *>(widget, 8));
    }
    if (select_combo_row(widget, row))
        events::post_widget(widget, 70, 0, 0);
}

void __fastcall combo_box_cancel(Widget *widget, void *) {
    edit_text_cancel(widget, nullptr);
    show_combo_list(widget, 0);
    select_combo_row(widget, find_combo_text(widget, invoke<const String *>(widget, 8)));
}

void __fastcall combo_box_configure_geometry(Widget *widget, void *, Dword source_address,
                                             Dword event, Dword first, Dword second) {
    Widget *list = combo_list(widget);
    if (!list)
        return;
    Widget *source = reinterpret_cast<Widget *>(source_address);
    if (source->id == at<Dword>(widget, 328)) {
        if (event == 43) {
            show_combo_list(widget, 1);
            activate(widget);
        }
        return;
    }
    if (source != list)
        return;
    if (event == 11) {
        if (invoke<Dword>(list, 9) || application::pending::captured_widget_id == list->id ||
            std::uint8_t(second))
            return;
    } else if (event == 12) {
        if (!invoke<Dword>(list, 9) && application::pending::captured_widget_id == list->id)
            return;
    } else if (event == 40) {
        const int selected = selected_list_row(&list_box(list)->items);
        at<int>(widget, 340) = selected;
        if (auto *item = invoke<ListItem *>(&list_box(list)->items, 9, selected, Dword(0))) {
            invoke(widget, 7, &item->text);
            events::post_widget(widget, 70, first, 0);
            show_combo_list(widget, 0);
            return;
        }
        show_combo_list(widget, 0);
        return;
    } else
        return;
    if (!first)
        show_combo_list(widget, 0);
}

Dword __fastcall combo_box_key_down(Widget *widget, void *, Dword key, Dword modifiers) {
    return key == 208 || key == 200 ? Dword(1)
                                      : edit_text_key_down(widget, nullptr, key, modifiers);
}
} // namespace nfs::gui::widgets::family
