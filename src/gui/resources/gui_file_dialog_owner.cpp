#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../widgets/gui_widget_shared_methods.hpp"
#include "gui_file_dialog.hpp"
#include "../../frontend/flow/frontend_application_widgets.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>
namespace nfs::gui::widgets::family {

static void destroy_file_dialog(FileDialog *dialog) {
    dialog->base.base.methods = file_dialog_methods.data();
    string_destruct(&dialog->filename);
    string_destruct(&dialog->directory);
    string_destruct(&dialog->filter);
    dialog->base.base.methods = dialog_methods;
    destroy_general(&dialog->base);
}

static FileDialog *__fastcall delete_file_dialog(FileDialog *dialog, void *, Dword flags) {
    destroy_file_dialog(dialog);
    if (flags & 1)
        release(dialog);
    return dialog;
}

const char *__cdecl file_dialog_class_key() {
    return "GFileDialog";
}

static const char *__fastcall file_dialog_name(FileDialog *, void *) {
    return "GFileDialog";
}

static Dword __fastcall file_dialog_is_type(FileDialog *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(file_dialog_class_key) ||
           type == reinterpret_cast<const void *>(dialog_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall file_dialog_is_name(FileDialog *, void *, const char *name) {
    return !std::strncmp(name, "GFileDialog", 12) || !std::strncmp(name, "GDialog", 8) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

static const String *__fastcall file_dialog_directory(FileDialog *dialog, void *) {
    return &dialog->directory;
}

static const String *__fastcall file_dialog_filename(FileDialog *dialog, void *) {
    return &dialog->filename;
}

const std::array<Method, 52> file_dialog_methods = {
    reinterpret_cast<Method>(::nfs::gui::widgets::family::delete_file_dialog), // 0.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::file_dialog_name), // 1.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::file_dialog_is_type), // 2.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::file_dialog_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_load), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_save), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::file_dialog_child), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::initialize_file_dialog), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_tick), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_key_down), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_button_down), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_pointer_press), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::accept_file_dialog), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_cancel), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::show_dialog), // 47.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::hide_dialog), // 48.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::file_dialog_directory), // 49.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::file_dialog_filename), // 50.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::refresh_file_dialog), // 51.
};

static void *allocate_file_dialog(void *parent) {
    auto *dialog = static_cast<FileDialog *>(allocate_flags(sizeof(FileDialog), 0, "GFileDialog"));
    if (!dialog)
        return nullptr;
    try {
        pending::construct_dialog(dialog, parent);
        string_construct(&dialog->filter);
        string_construct(&dialog->directory);
        string_construct(&dialog->filename);
    } catch (...) {
        release_flags(dialog, 0, "GFileDialog");
        throw;
    }
    dialog->list_id = dialog->attributes = dialog->sort_mode = 0;
    dialog->base.base.flags |= 131;
    dialog->base.base.methods = file_dialog_methods.data();
    return dialog;
}
static ClassRecord file_dialog_class;

static void unregister_file_dialog() {
    unregister_class(&file_dialog_class);
}

static void register_file_dialog() {
    register_class(&file_dialog_class, "GFileDialog", allocate_file_dialog, "GDialog");
    std::atexit(unregister_file_dialog);
}
} // namespace nfs::gui::widgets::family
NFS_KERNEL_INITIALIZER(startup_register_file_dialog, ".CRT$XCU_NFS_006906F8",
                       nfs::gui::widgets::family::register_file_dialog);
