#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_network_pages.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;

static ServerSetup *construct_server_setup(ServerSetup *page, void *parent) {
    gui::widgets::pending::construct_dialog(page, parent);
    page->base.methods = server_setup_methods.data();
    page->accept = page->cancel = page->dedicated = page->name = page->password = nullptr;
    page->track = page->direction = page->race_type = page->car_class = page->laps =
        page->maximum_clients = page->collisions = nullptr;
    return page;
}

static void destroy_server_setup(ServerSetup *page) {
    page->base.methods = server_setup_methods.data();
    family::dialog_destroy(&page->base);
}

static ServerSetup *__fastcall delete_server_setup(ServerSetup *page, void *, Dword flags) {
    destroy_server_setup(page);
    if (flags & 1)
        gui::release(page);
    return page;
}

const char *__cdecl server_setup_class_key() {
    return "ServerSetup";
}

static const char *__fastcall server_setup_name(ServerSetup *, void *) {
    return "ServerSetup";
}

static Dword __fastcall server_setup_is_type(ServerSetup *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(server_setup_class_key) ||
           type == reinterpret_cast<const void *>(family::dialog_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall server_setup_is_name(ServerSetup *, void *, const char *name) {
    return !std::strncmp(name, "ServerSetup", 12) || !std::strncmp(name, "GDialog", 8) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 49> server_setup_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_server_setup), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::server_setup_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::server_setup_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::server_setup_is_name), // 3.
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
    reinterpret_cast<Method>(::nfs::application::frontend::server_setup_child), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_server_setup), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::tick_server_setup), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::application::frontend::close_server_setup), // 31.
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
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_accept), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_cancel), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::show_dialog), // 47.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::hide_dialog), // 48.
};

static void *create_server_setup(void *parent) {
    auto *page =
        static_cast<ServerSetup *>(gui::allocate_flags(sizeof(ServerSetup), 0, "ServerSetup"));
    if (!page)
        return nullptr;
    try {
        return construct_server_setup(page, parent);
    } catch (...) {
        gui::release_flags(page, 0, "ServerSetup");
        throw;
    }
}
static ClassRecord server_setup_class;

static void unregister_server_setup() {
    unregister_class(&server_setup_class);
}

static void register_server_setup() {
    register_class(&server_setup_class, "ServerSetup", create_server_setup, "GDialog");
    std::atexit(unregister_server_setup);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_server_setup, ".CRT$XCU_NFS_006900F0",
                       register_server_setup);
} // namespace

static ClientCarSelection *construct_client_car_selection(ClientCarSelection *page, void *parent) {
    gui::widgets::pending::construct_dialog(page, parent);
    page->base.methods = client_car_selection_methods.data();
    page->select = page->cancel = page->observe = nullptr;
    page->transmission = nullptr;
    page->colours = nullptr;
    page->cars = nullptr;
    return page;
}

static void destroy_client_car_selection(ClientCarSelection *page) {
    page->base.methods = client_car_selection_methods.data();
    family::dialog_destroy(&page->base);
}

static ClientCarSelection *__fastcall delete_client_car_selection(ClientCarSelection *page, void *,
                                                                  Dword flags) {
    destroy_client_car_selection(page);
    if (flags & 1)
        gui::release(page);
    return page;
}

const char *__cdecl client_car_selection_class_key() {
    return "ClientCarSelect";
}

static const char *__fastcall client_car_selection_name(ClientCarSelection *, void *) {
    return "ClientCarSelect";
}

static Dword __fastcall client_car_selection_is_type(ClientCarSelection *, void *,
                                                    const void *type) {
    return type == reinterpret_cast<const void *>(client_car_selection_class_key) ||
           type == reinterpret_cast<const void *>(family::dialog_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall client_car_selection_is_name(ClientCarSelection *, void *,
                                                    const char *name) {
    return !std::strncmp(name, "ClientCarSelect", 16) || !std::strncmp(name, "GDialog", 8) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 49> client_car_selection_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_client_car_selection), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::client_car_selection_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::client_car_selection_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::client_car_selection_is_name), // 3.
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
    reinterpret_cast<Method>(::nfs::application::frontend::client_car_selection_child), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_client_car_selection), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::tick_client_car_selection), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::application::frontend::close_client_car_selection), // 31.
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
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_accept), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_cancel), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::show_dialog), // 47.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::hide_dialog), // 48.
};

static void *create_client_car_selection(void *parent) {
    auto *page = static_cast<ClientCarSelection *>(
        gui::allocate_flags(sizeof(ClientCarSelection), 0, "ClientCarSelect"));
    if (!page)
        return nullptr;
    try {
        return construct_client_car_selection(page, parent);
    } catch (...) {
        gui::release(page);
        throw;
    }
}
static ClassRecord client_car_selection_class;

static void unregister_client_car_selection() {
    unregister_class(&client_car_selection_class);
}

static void register_client_car_selection() {
    register_class(&client_car_selection_class, "ClientCarSelect", create_client_car_selection,
                   "GDialog");
    std::atexit(unregister_client_car_selection);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_client_car_selection, ".CRT$XCU_NFS_0069022C",
                       register_client_car_selection);
} // namespace

static NetworkStatistics *construct_network_statistics(NetworkStatistics *page, void *parent) {
    gui::widgets::pending::construct_dialog(page, parent);
    page->base.methods = network_statistics_methods.data();
    page->phase = page->status = page->rotation = page->race_type = page->server_name =
        page->car_class = page->track = page->direction = nullptr;
    page->players = nullptr;
    page->rotation_deadline = 0;
    page->next_column = 0;
    return page;
}

static void destroy_network_statistics(NetworkStatistics *page) {
    page->base.methods = network_statistics_methods.data();
    family::dialog_destroy(&page->base);
}

static NetworkStatistics *__fastcall delete_network_statistics(NetworkStatistics *page, void *,
                                                               Dword flags) {
    destroy_network_statistics(page);
    if (flags & 1)
        gui::release(page);
    return page;
}

const char *__cdecl network_statistics_class_key() {
    return "NetStats";
}

static const char *__fastcall network_statistics_name(NetworkStatistics *, void *) {
    return "NetStats";
}

static Dword __fastcall network_statistics_is_type(NetworkStatistics *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(network_statistics_class_key) ||
           type == reinterpret_cast<const void *>(family::dialog_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall network_statistics_is_name(NetworkStatistics *, void *, const char *name) {
    return !std::strncmp(name, "NetStats", 9) || !std::strncmp(name, "GDialog", 8) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 49> network_statistics_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_network_statistics), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::network_statistics_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::network_statistics_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::network_statistics_is_name), // 3.
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
    reinterpret_cast<Method>(::nfs::application::frontend::network_statistics_child), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_network_statistics), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::tick_network_statistics), // 24.
    reinterpret_cast<Method>(::nfs::application::frontend::clear_network_statistics_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::application::frontend::network_statistics_key), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_button_down), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_pointer_press), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_accept), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_cancel), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::show_dialog), // 47.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::hide_dialog), // 48.
};

static void *create_network_statistics(void *parent) {
    auto *page = static_cast<NetworkStatistics *>(
        gui::allocate_flags(sizeof(NetworkStatistics), 0, "NetStats"));
    if (!page)
        return nullptr;
    try {
        return construct_network_statistics(page, parent);
    } catch (...) {
        gui::release(page);
        throw;
    }
}
static ClassRecord network_statistics_class;

static void unregister_network_statistics() {
    unregister_class(&network_statistics_class);
}

static void register_network_statistics() {
    register_class(&network_statistics_class, "NetStats", create_network_statistics, "GDialog");
    std::atexit(unregister_network_statistics);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_network_statistics, ".CRT$XCU_NFS_00690278",
                       register_network_statistics);
} // namespace

static NetworkOptions *construct_network_options(NetworkOptions *page, void *parent) {
    gui::widgets::pending::construct_dialog(page, parent);
    page->base.methods = network_options_methods.data();
    page->server = page->car = page->hud = page->audio = page->disconnect = page->close = nullptr;
    return page;
}

static void destroy_network_options(NetworkOptions *page) {
    page->base.methods = network_options_methods.data();
    family::dialog_destroy(&page->base);
}

static NetworkOptions *__fastcall delete_network_options(NetworkOptions *page, void *, Dword flags) {
    destroy_network_options(page);
    if (flags & 1)
        gui::release(page);
    return page;
}

const char *__cdecl network_options_class_key() {
    return "NetOptions";
}

static const char *__fastcall network_options_name(NetworkOptions *, void *) {
    return "NetOptions";
}

static Dword __fastcall network_options_is_type(NetworkOptions *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(network_options_class_key) ||
           type == reinterpret_cast<const void *>(family::dialog_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall network_options_is_name(NetworkOptions *, void *, const char *name) {
    return !std::strncmp(name, "NetOptions", 11) || !std::strncmp(name, "GDialog", 8) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 49> network_options_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_network_options), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::network_options_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::network_options_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::network_options_is_name), // 3.
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
    reinterpret_cast<Method>(::nfs::application::frontend::network_options_child), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_network_options), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::tick_network_options), // 24.
    reinterpret_cast<Method>(::nfs::application::frontend::leave_network_options), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::application::frontend::close_network_options), // 31.
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
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_accept), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_cancel), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::show_dialog), // 47.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::hide_dialog), // 48.
};

static void *create_network_options(void *parent) {
    auto *page =
        static_cast<NetworkOptions *>(gui::allocate_flags(sizeof(NetworkOptions), 0, "NetOptions"));
    if (!page)
        return nullptr;
    try {
        return construct_network_options(page, parent);
    } catch (...) {
        gui::release(page);
        throw;
    }
}
static ClassRecord network_options_class;

static void unregister_network_options() {
    unregister_class(&network_options_class);
}

static void register_network_options() {
    register_class(&network_options_class, "NetOptions", create_network_options, "GDialog");
    std::atexit(unregister_network_options);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_network_options, ".CRT$XCU_NFS_0069027C",
                       register_network_options);
} // namespace
} // namespace nfs::application::frontend
