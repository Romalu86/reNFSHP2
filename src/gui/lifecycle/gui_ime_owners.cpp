#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../widgets/gui_widget_shared_methods.hpp"
#include "gui_ime.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>
namespace nfs::gui::widgets::ime {

static Edit *construct_edit(Edit *edit, void *parent) {
    pending::construct_edit_text(edit, parent);
    edit->composition_caret = 0;
    edit->language = 0;
    try {
        string_construct(&edit->status);
    } catch (...) {
        family::edit_text_destroy(reinterpret_cast<Widget *>(edit));
        throw;
    }
    try {
        edit->properties = 0;
        edit->pending_character = 0;
        edit->composing = 0;
        edit->wide_os = 1;
        edit->disabled = 0;
        edit->candidate_id = 0;
        edit->base.base.base.methods = edit_methods.data();
        OSVERSIONINFOA version;
        version.dwOSVersionInfoSize = sizeof(version);
        GetVersionExA(&version);
        edit->wide_os = version.dwPlatformId == VER_PLATFORM_WIN32_NT && version.dwMajorVersion >= 4;
        if (!instance_count) {
            install_message_hook();
            context = ImmCreateContext();
            if (context) {
                ImmAssociateContext(input_window(), context);
                edit->detached_context = context;
                enable_context(edit);
                ImmSetOpenStatus(context, FALSE);
            }
        }
        ++instance_count;
        edit->base.alphabet = nullptr;
        edit->base.alphabet_position = 0;
    } catch (...) {
        string_destruct(&edit->status);
        family::edit_text_destroy(reinterpret_cast<Widget *>(edit));
        throw;
    }
    return edit;
}

static void destroy_edit(Edit *edit) {
    edit->base.base.base.methods = edit_methods.data();
    try {
        if (!--instance_count) {
            close_context(edit);
            detach_context(edit);
            remove_message_hook();
            if (context) {
                ImmDestroyContext(context);
                context = nullptr;
            }
        }
    } catch (...) {
        string_destruct(&edit->status);
        family::edit_text_destroy(reinterpret_cast<Widget *>(edit));
        throw;
    }
    try {
        string_destruct(&edit->status);
    } catch (...) {
        family::edit_text_destroy(reinterpret_cast<Widget *>(edit));
        throw;
    }
    family::edit_text_destroy(reinterpret_cast<Widget *>(edit));
}

static Edit *__fastcall delete_edit(Edit *edit, void *, Dword flags) {
    destroy_edit(edit);
    if (flags & 1)
        release(edit);
    return edit;
}

static const char *__fastcall edit_name(Edit *, void *) {
    return "GIME";
}

const char *__cdecl edit_class_key() {
    return "GIME";
}

static Dword __fastcall edit_is_name(Edit *, void *, const char *name) {
    return !std::strncmp(name, "GIME", 5) || !std::strncmp(name, "GEditText", 10) ||
           !std::strncmp(name, "GText", 6) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

static Dword __fastcall edit_is_type(Edit *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(edit_class_key) ||
           type == reinterpret_cast<const void *>(family::edit_text_class_key) ||
           type == reinterpret_cast<const void *>(family::text_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

const std::array<Method, 59> edit_methods = {
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::delete_edit), // 0.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::edit_name), // 1.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::edit_is_type), // 2.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::edit_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_load), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_save), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::text_layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_draw), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::request_general_layout), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_key_down), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::character), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_button_down), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_pointer_move), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_pointer_press), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_pointer_release), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_accept), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_cancel), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::draw_edit_line), // 47.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::reject_edit_character), // 48.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::begin_edit_session), // 49.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_item_operation_50), // 50.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::edit_operation), // 51.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::validate_edit_text), // 52.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_item_operation_53), // 53.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::draw_caret), // 54.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::update_composition), // 55.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::begin_composition), // 56.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::end_composition), // 57.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::filter_character), // 58.
};

static void *create_edit(void *parent) {
    auto *edit = static_cast<Edit *>(allocate_flags(sizeof(Edit), 0, "GIME"));
    if (!edit)
        return nullptr;
    try {
        return construct_edit(edit, parent);
    } catch (...) {
        release_flags(edit, 0, "GIME");
        throw;
    }
}
static ClassRecord edit_class;

static void unregister_edit() {
    unregister_class(&edit_class);
}

static void register_edit() {
    register_class(&edit_class, edit_class_key(), create_edit, "GEditText");
}

static void retain_edit_finalizer() {
    std::atexit(unregister_edit);
}

static void initialize_edit_class() {
    register_edit();
    retain_edit_finalizer();
}
} // namespace nfs::gui::widgets::ime
NFS_KERNEL_INITIALIZER(startup_initialize_edit_class, ".CRT$XCU_NFS_00690090",
                       nfs::gui::widgets::ime::initialize_edit_class);
