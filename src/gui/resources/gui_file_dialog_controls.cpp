#include "gui_file_dialog.hpp"
#include "../../frontend/flow/frontend_application_widgets.hpp"
namespace nfs::application::pending {
void *create_widget(const gui::String *, const gui::String *, const gui::String *, void *);
}
namespace nfs::gui::widgets::family {
using game::runtime::invoke;

void __fastcall initialize_file_dialog(FileDialog *dialog, void *) {
    dialog->list_id = named_child_id(&dialog->base.base, "<FILELIST>");
    dialog->accept_id = named_child_id(&dialog->base.base, "<OK>");
    dialog->cancel_id = named_child_id(&dialog->base.base, "<CANCEL>");
    pending::request_general_layout(&dialog->base, nullptr);
}

FileDialog *create_file_dialog(const String *resource, const String *name, const String *type,
                               void *parent, const String *directory, const String *filter,
                               const String *filename, Dword attributes, Dword sort) {
    auto *widget =
        static_cast<Widget *>(application::pending::create_widget(resource, name, type, parent));
    if (!widget)
        return nullptr;
    if (!invoke<Dword>(widget, 2, file_dialog_class_key)) {
        application::pending::destroy_screen(widget);
        return nullptr;
    }
    auto *dialog = reinterpret_cast<FileDialog *>(widget);
    dialog->attributes = attributes;
    string_copy(&dialog->filter, filter);
    string_copy(&dialog->directory, directory);
    string_copy(&dialog->filename, filename);
    dialog->sort_mode = sort;
    invoke(dialog, 51);
    return dialog;
}

void __fastcall file_dialog_child(FileDialog *dialog, void *, Widget *sender, Dword event,
                                  Dword first, Dword) {
    auto *list = sender && invoke<Dword>(sender, 2, list_box_class_key)
                     ? reinterpret_cast<ListBox *>(sender)
                     : nullptr;
    if (list && sender->id == dialog->list_id && event == 40) {
        auto *items = &list->items;
        const int row = selected_list_row(items);
        auto *item = invoke<ListItem *>(items, 9, Dword(row), Dword(0));
        if (!item)
            return;
        if (!(item->data & 16)) {
            string_copy(&dialog->filename, &item->text);
            events::post_widget(dialog, 40, first, 0);
        } else {
            string_append_ascii(&dialog->directory, "\\");
            string_append(&dialog->directory, &item->text);
            invoke(dialog, 51);
        }
        return;
    }
    if (event == 43) {
        if (sender->id == dialog->accept_id)
            events::post_widget(dialog, 40, first, 0);
        else if (sender->id == dialog->cancel_id)
            events::post_widget(dialog, 41, first, 0);
    }
}

void __fastcall accept_file_dialog(FileDialog *dialog, void *) {
    finish_modal(&dialog->base.base, 2, dialog->base.base.id);
}

void __fastcall refresh_file_dialog(FileDialog *dialog, void *) {
    auto *widget = dialog->list_id
                       ? static_cast<Widget *>(events::find_widget(context_state, dialog->list_id))
                       : nullptr;
    if (!widget || !invoke<Dword>(widget, 2, list_box_class_key))
        return;
    auto *list = reinterpret_cast<ListBox *>(widget);
    invoke(&list->items, 1);
    populate_directory_list(list, 0, &dialog->directory, &dialog->filter, 4, dialog->attributes,
                            nullptr, 0);
    if (dialog->sort_mode & 2)
        sort_list_items(&list->items, 0, compare_file_extensions);
    else if (dialog->sort_mode & 1)
        sort_list_items(&list->items, 0, compare_file_names);
}
} // namespace nfs::gui::widgets::family
