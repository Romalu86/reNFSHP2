#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_menu_controls.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstring>
namespace nfs::application::pending {
const char *__cdecl typewriter_class_key();
}
namespace nfs::application::frontend {
using namespace gui::widgets;
void __fastcall update_rollover(RolloverButton *, void *, Dword);
Dword __fastcall rollover_pointer_move(RolloverButton *, void *, Dword, Dword);
Dword __fastcall rollover_state(RolloverButton *, void *);
void __fastcall activate_rollover(RolloverButton *, void *);


const char *__cdecl type_spin_class_key() {
    return "TypeSpinCtl";
}

static const char *__fastcall type_spin_name(Widget *, void *) {
    return "TypeSpinCtl";
}

static Dword __fastcall type_spin_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(type_spin_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall type_spin_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "TypeSpinCtl", 12) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

static TypeSpinControl *construct_type_spin(TypeSpinControl *widget, void *parent) {
    construct_widget(widget, parent);
    gui::string_construct(&widget->list_name);
    widget->binding = 0;
    try {
        family::construct_list_items(&widget->items);
    } catch (...) {
        gui::string_destruct(&widget->list_name);
        destroy_general(&widget->base);
        throw;
    }
    reinterpret_cast<Widget *>(widget)->methods = type_spin_methods.data();
    try {
        gui::string_copy(&widget->list_name,
                         &lifecycle::construction::unresolved::shared_string);
    } catch (...) {
        family::destroy_list_items(&widget->items);
        gui::string_destruct(&widget->list_name);
        destroy_general(&widget->base);
        throw;
    }
    widget->display = nullptr;
    widget->next_button = widget->previous_button = 0;
    return widget;
}

static void destroy_type_spin(TypeSpinControl *widget) {
    reinterpret_cast<Widget *>(widget)->methods = type_spin_methods.data();
    family::destroy_list_items(&widget->items);
    gui::string_destruct(&widget->list_name);
    destroy_general(reinterpret_cast<GeneralWidget *>(widget));
}

static TypeSpinControl *__fastcall delete_type_spin(TypeSpinControl *widget, void *, Dword flags) {
    destroy_type_spin(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

const std::array<Method, 47> type_spin_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_type_spin), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::type_spin_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::type_spin_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::type_spin_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::load_type_spin), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::save_type_spin), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::type_spin_event), // 13.
    reinterpret_cast<Method>(::nfs::application::frontend::type_spin_child_event), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::application::frontend::draw_type_spin), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_type_spin), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_key), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_button), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_focus_style), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::accept_general), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::cancel_general), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
};

void *create_type_spin(void *parent) {
    auto *storage = static_cast<TypeSpinControl *>(
        gui::allocate_flags(sizeof(TypeSpinControl), 0, "TypeSpinCtl"));
    if (!storage)
        return nullptr;
    try {
        return construct_type_spin(storage, parent);
    } catch (...) {
        gui::release_flags(storage, 0, "TypeSpinCtl");
        throw;
    }
}

const char *__cdecl options_view_class_key() {
    return "HP2OptionsView";
}

static const char *__fastcall options_view_name(Widget *, void *) {
    return "HP2OptionsView";
}

static Dword __fastcall options_view_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(options_view_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall options_view_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "HP2OptionsView", 15) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

static OptionsView *construct_options_view(OptionsView *widget, void *parent) {
    construct_widget(widget, parent);
    widget->selected_tab = -1;
    widget->content = nullptr;
    reinterpret_cast<Widget *>(widget)->methods = options_view_methods.data();
    return widget;
}

static void destroy_options_view(OptionsView *widget) {
    reinterpret_cast<Widget *>(widget)->methods = options_view_methods.data();
    destroy_general(reinterpret_cast<GeneralWidget *>(widget));
}

static OptionsView *__fastcall delete_options_view(OptionsView *widget, void *, Dword flags) {
    destroy_options_view(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

const std::array<Method, 47> options_view_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_options_view), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::options_view_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::options_view_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::options_view_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_general), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_general), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_options_view), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::update_options_view), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_key), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_button), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_focus_style), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::accept_general), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::cancel_general), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
};

void *create_options_view(void *parent) {
    auto *storage =
        static_cast<OptionsView *>(gui::allocate_flags(sizeof(OptionsView), 0, "HP2OptionsView"));
    if (!storage)
        return nullptr;
    try {
        return construct_options_view(storage, parent);
    } catch (...) {
        gui::release_flags(storage, 0, "HP2OptionsView");
        throw;
    }
}

const char *__cdecl challenge_view_class_key() {
    return "HP2SingleChallengeView";
}

static const char *__fastcall challenge_view_name(Widget *, void *) {
    return "HP2SingleChallengeView";
}

static Dword __fastcall challenge_view_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(challenge_view_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall challenge_view_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "HP2SingleChallengeView", 23) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

static ChallengeView *construct_challenge_view(ChallengeView *widget, void *parent) {
    construct_widget(widget, parent);
    widget->selected = 0;
    widget->content = nullptr;
    reinterpret_cast<Widget *>(widget)->methods = challenge_view_methods.data();
    return widget;
}

static void destroy_challenge_view(ChallengeView *widget) {
    reinterpret_cast<Widget *>(widget)->methods = challenge_view_methods.data();
    destroy_general(reinterpret_cast<GeneralWidget *>(widget));
}

static ChallengeView *__fastcall delete_challenge_view(ChallengeView *widget, void *, Dword flags) {
    destroy_challenge_view(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

const std::array<Method, 47> challenge_view_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_challenge_view), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::challenge_view_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::challenge_view_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::challenge_view_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_general), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_general), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::challenge_view_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_challenge_view), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::update_challenge_view), // 24.
    reinterpret_cast<Method>(::nfs::application::frontend::release_challenge_view), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_key), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_button), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_focus_style), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::accept_general), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::cancel_general), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
};

void *create_challenge_view(void *parent) {
    auto *storage = static_cast<ChallengeView *>(
        gui::allocate_flags(sizeof(ChallengeView), 0, "HP2SingleChallengeView"));
    if (!storage)
        return nullptr;
    try {
        return construct_challenge_view(storage, parent);
    } catch (...) {
        gui::release_flags(storage, 0, "HP2SingleChallengeView");
        throw;
    }
}

const char *__cdecl menu_spin_class_key() {
    return "HP2SpinButton";
}

static const char *__fastcall menu_spin_name(Widget *, void *) {
    return "HP2SpinButton";
}

static Dword __fastcall menu_spin_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(menu_spin_class_key) ||
           type == reinterpret_cast<const void *>(family::button_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall menu_spin_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "HP2SpinButton", 14) || !std::strncmp(name, "GButton", 8) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

static MenuSpinButton *construct_menu_spin(MenuSpinButton *widget, void *parent) {
    gui::widgets::pending::construct_button(widget, parent);
    widget->highlight_duration = 0;
    widget->highlight_ticks = 0;
    reinterpret_cast<Widget *>(widget)->methods = menu_spin_methods.data();
    return widget;
}

static void destroy_menu_spin(MenuSpinButton *widget) {
    reinterpret_cast<Widget *>(widget)->methods = menu_spin_methods.data();
    family::button_destroy(reinterpret_cast<Widget *>(widget));
}

static MenuSpinButton *__fastcall delete_menu_spin(MenuSpinButton *widget, void *, Dword flags) {
    destroy_menu_spin(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

const std::array<Method, 52> menu_spin_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_menu_spin), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_spin_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_spin_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_spin_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::load_menu_spin), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::save_menu_spin), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_initialize), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::update_menu_spin), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_key_down), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_button_down), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_pointer_press), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_pointer_release), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::accept_general), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::cancel_general), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::set_button_checked), // 47.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_spin_state), // 48.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::activate_button), // 49.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_item_operation_50), // 50.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_set_checked_event), // 51.
};

void *create_menu_spin(void *parent) {
    auto *storage = static_cast<MenuSpinButton *>(
        gui::allocate_flags(sizeof(MenuSpinButton), 0, "HP2SpinButton"));
    if (!storage)
        return nullptr;
    try {
        return construct_menu_spin(storage, parent);
    } catch (...) {
        gui::release_flags(storage, 0, "HP2SpinButton");
        throw;
    }
}

static const char *__fastcall typewriter_name(Widget *, void *) {
    return "HP2TypeWriter";
}

static Dword __fastcall typewriter_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(pending::typewriter_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall typewriter_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "HP2TypeWriter", 14) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

static TypeWriter *construct_typewriter(TypeWriter *widget, void *parent) {
    gui::widgets::pending::construct_edit_text(widget, parent);
    widget->wheel = nullptr;
    widget->previous_button = widget->next_button = 0;
    reinterpret_cast<Widget *>(widget)->methods = typewriter_methods.data();
    return widget;
}

static void destroy_typewriter(TypeWriter *widget) {
    reinterpret_cast<Widget *>(widget)->methods = typewriter_methods.data();
    family::edit_text_destroy(reinterpret_cast<Widget *>(widget));
}

static TypeWriter *__fastcall delete_typewriter(TypeWriter *widget, void *, Dword flags) {
    destroy_typewriter(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

const std::array<Method, 55> typewriter_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_typewriter), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::typewriter_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::typewriter_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::typewriter_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_load), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_save), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::text_layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::typewriter_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::application::frontend::draw_typewriter), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_typewriter), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_key_down), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_character), // 35.
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
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_item_operation_51), // 51.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::validate_edit_text), // 52.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_item_operation_53), // 53.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::draw_edit_caret), // 54.
};

void *create_typewriter(void *parent) {
    auto *storage =
        static_cast<TypeWriter *>(gui::allocate_flags(sizeof(TypeWriter), 0, "HP2TypeWriter"));
    if (!storage)
        return nullptr;
    try {
        return construct_typewriter(storage, parent);
    } catch (...) {
        gui::release_flags(storage, 0, "HP2TypeWriter");
        throw;
    }
}

const char *__cdecl text_wheel_class_key() {
    return "HP2TypeWriterText";
}

static const char *__fastcall text_wheel_name(Widget *, void *) {
    return "HP2TypeWriterText";
}

static Dword __fastcall text_wheel_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(text_wheel_class_key) ||
           type == reinterpret_cast<const void *>(family::text_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall text_wheel_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "HP2TypeWriterText", 18) || !std::strncmp(name, "GText", 6) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

static TextWheel *construct_text_wheel(TextWheel *widget, void *parent) {
    gui::widgets::pending::construct_text(widget, parent);
    widget->character_pitch = 20;
    widget->scroll_offset = 0;
    widget->selected_character = 0;
    reinterpret_cast<Widget *>(widget)->methods = text_wheel_methods.data();
    return widget;
}

static void destroy_text_wheel(TextWheel *widget) {
    reinterpret_cast<Widget *>(widget)->methods = text_wheel_methods.data();
    family::text_destroy(reinterpret_cast<Widget *>(widget));
}

static TextWheel *__fastcall delete_text_wheel(TextWheel *widget, void *, Dword flags) {
    destroy_text_wheel(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

const std::array<Method, 48> text_wheel_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_text_wheel), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::text_wheel_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::text_wheel_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::text_wheel_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::load_text_wheel), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::save_text_wheel), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::text_layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::application::frontend::draw_text_wheel), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::request_general_layout), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::update_text_wheel), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_key), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_button), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_focus_style), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::accept_general), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::cancel_general), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::draw_text_line), // 47.
};

void *create_text_wheel(void *parent) {
    auto *storage =
        static_cast<TextWheel *>(gui::allocate_flags(sizeof(TextWheel), 0, "HP2TypeWriterText"));
    if (!storage)
        return nullptr;
    try {
        return construct_text_wheel(storage, parent);
    } catch (...) {
        gui::release_flags(storage, 0, "HP2TypeWriterText");
        throw;
    }
}

const char *__cdecl rollover_class_key() {
    return "RolloverButton";
}

static const char *__fastcall rollover_name(Widget *, void *) {
    return "RolloverButton";
}

static Dword __fastcall rollover_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(rollover_class_key) ||
           type == reinterpret_cast<const void *>(family::button_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall rollover_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "RolloverButton", 15) || !std::strncmp(name, "GButton", 8) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

static RolloverButton *construct_rollover(RolloverButton *widget, void *parent) {
    gui::widgets::pending::construct_button(widget, parent);
    widget->latched = 0;
    reinterpret_cast<Widget *>(widget)->methods = rollover_methods.data();
    return widget;
}

static void destroy_rollover(RolloverButton *widget) {
    reinterpret_cast<Widget *>(widget)->methods = rollover_methods.data();
    family::button_destroy(reinterpret_cast<Widget *>(widget));
}

static RolloverButton *__fastcall delete_rollover(RolloverButton *widget, void *, Dword flags) {
    destroy_rollover(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

const std::array<Method, 52> rollover_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_rollover), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::rollover_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::rollover_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::rollover_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_load), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_save), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_initialize), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::update_rollover), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_key_down), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_button_down), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::application::frontend::rollover_pointer_move), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_pointer_press), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_pointer_release), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::accept_general), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::cancel_general), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::set_button_checked), // 47.
    reinterpret_cast<Method>(::nfs::application::frontend::rollover_state), // 48.
    reinterpret_cast<Method>(::nfs::application::frontend::activate_rollover), // 49.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_item_operation_50), // 50.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_set_checked_event), // 51.
};

void *create_rollover(void *parent) {
    auto *storage = static_cast<RolloverButton *>(
        gui::allocate_flags(sizeof(RolloverButton), 0, "RolloverButton"));
    if (!storage)
        return nullptr;
    try {
        return construct_rollover(storage, parent);
    } catch (...) {
        gui::release_flags(storage, 0, "RolloverButton");
        throw;
    }
}
} // namespace nfs::application::frontend
