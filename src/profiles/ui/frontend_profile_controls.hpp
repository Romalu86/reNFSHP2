#pragma once
#include "../../frontend/screens/frontend_menu_pages.hpp"
#include "../../gui/widgets/gui_widget_list.hpp"
#include "../../input/controllers/controller_interfaces.hpp"
namespace nfs::gui::widgets::family {
void set_slider_bar_position(SliderBar *, int);
}
namespace nfs::application::frontend {
struct ControllerPage {
    GeneralWidget base;
    void *preferences;
    Dword label_column, binding_column;
    gui::widgets::family::ListBox *list;
    Dword bindings[26];
    std::uint8_t has_wheel, padding[3];
};
struct DeadzoneDialog {
    Widget base;
    std::uint8_t dialog_members[32];
    Dword axis_count;
};
struct ControlRemapDialog {
    Widget base;
    std::uint8_t message_members[40];
    std::uint8_t released, unused_129, padding[2];
    Dword elapsed;
    void *input;
    Dword snapshots[16][65];
    Dword binding, value;
};
struct DriverProfilePage {
    GeneralWidget base;
    Widget *new_button, *delete_button, *load_button, *save_button, *current_name;
    Dword reserved_11c;
    gui::widgets::family::ListBox *statistics;
    ProfileToggle *category;
    void *preferences, *events, *cars;
};
static_assert(sizeof(ControllerPage) == 388 && offsetof(ControllerPage, bindings) == 280);
static_assert(sizeof(DeadzoneDialog) == 292 && sizeof(ControlRemapDialog) == 4476);
static_assert(offsetof(ControlRemapDialog, binding) == 4468);
static_assert(sizeof(DriverProfilePage) == 308);
extern const std::array<Method, 47> controller_page_methods, driver_profile_methods;
extern const std::array<Method, 49> deadzone_methods, control_remap_methods;
const char *__cdecl controller_page_class_key();
const char *__cdecl deadzone_class_key();
const char *__cdecl control_remap_class_key();
const char *__cdecl driver_profile_class_key();
ControllerPage *construct_controller_page(ControllerPage *, void *);
void __fastcall initialize_controller_page(ControllerPage *, void *);
void __fastcall release_controller_page(ControllerPage *, void *);
void __fastcall controller_page_child_event(ControllerPage *, void *, Widget *, Dword, Dword, Dword);
void __fastcall controller_page_command(ControllerPage *, void *, Dword, Dword);
void refresh_controller_bindings(ControllerPage *);
Dword describe_menu_binding(void *, gui::String *);
const gui::String *menu_list_text(gui::widgets::family::ListItems *, Dword, Dword);
Dword menu_list_data(gui::widgets::family::ListItems *, Dword, Dword);
void save_current_controller_profile();
void modify_controller_binding(ControllerPage *);
bool assign_controller_binding(ControllerPage *, Dword, Dword *, Dword);
void *resolve_menu_control(void *, const Dword *, Dword, bool);
bool bindings_equal(const Dword *, const Dword *);
Dword run_deadzone_dialog(DeadzoneDialog *, Dword *, int);
Dword open_deadzone_dialog(Dword *, int);
void open_force_feedback_dialog(Widget *);
void __fastcall initialize_deadzone_dialog(DeadzoneDialog *, void *);
void __fastcall initialize_control_remap(ControlRemapDialog *, void *);
void __fastcall update_control_remap(ControlRemapDialog *, void *, Dword);
Dword __fastcall accept_control_remap(ControlRemapDialog *, void *, Dword, Dword);
void __fastcall initialize_driver_profile(DriverProfilePage *, void *);
void __fastcall release_driver_profile(DriverProfilePage *, void *);
void __fastcall driver_profile_child_event(DriverProfilePage *, void *, Widget *, Dword, Dword, Dword);
void refresh_profile_statistics(DriverProfilePage *);
void create_page_profile(DriverProfilePage *);
void delete_page_profile(DriverProfilePage *);
void load_page_profile(DriverProfilePage *);
void save_page_profile(DriverProfilePage *);
Dword enumerate_page_profiles(const char *, ProfileToggle *);
std::uint8_t save_named_profile(const char *);
Dword delete_profile_files(const char *);
void remember_profile(gui::String);
void append_profile_overview(DriverProfilePage *);
void append_profile_car_speed(DriverProfilePage *, Dword, float);
void format_profile_speed(gui::String *, float);
Dword count_profile_results(void *, Dword, Dword);
} // namespace nfs::application::frontend
