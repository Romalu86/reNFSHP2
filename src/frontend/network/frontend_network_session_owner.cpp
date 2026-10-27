#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_network_session.hpp"
#include "nfs/resources/localization/region.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;

static void destroy_network_session(NetworkSessionWidget *widget) {
    widget->base.base.methods = network_session_methods.data();
    destroy_general(&widget->base);
}

static NetworkSessionWidget *__fastcall delete_network_session(NetworkSessionWidget *widget, void *,
                                                               Dword flags) {
    destroy_network_session(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

static const char *__fastcall network_session_name(NetworkSessionWidget *, void *) {
    return "NetgameFE";
}

static Dword __fastcall network_session_is_type(NetworkSessionWidget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(pending::netgame_frontend_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall network_session_is_name(NetworkSessionWidget *, void *, const char *name) {
    return !std::strncmp(name, "NetgameFE", 10) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

static Dword __fastcall network_session_event(NetworkSessionWidget *widget, void *, Dword event,
                                             Dword first, Dword second) {
    return gui::widgets::pending::general_event(&widget->base, nullptr, event, first, second);
}
// RET10 has no side effects.
static void __fastcall network_session_child(NetworkSessionWidget *, void *, Widget *, Dword, Dword,
                                             Dword) {
}

const std::array<Method, 47> network_session_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_network_session), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::network_session_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::network_session_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::network_session_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_general), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_general), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::network_session_event), // 13.
    reinterpret_cast<Method>(::nfs::application::frontend::network_session_child), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_network_session), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::tick_network_session), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::application::frontend::network_session_key), // 32.
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

static void *create_network_session(void *parent) {
    auto *widget = static_cast<NetworkSessionWidget *>(
        gui::allocate_flags(sizeof(NetworkSessionWidget), 0, "NetgameFE"));
    if (!widget)
        return nullptr;
    try {
        construct_widget(widget, parent);
    } catch (...) {
        gui::release(widget);
        throw;
    }
    widget->mode = widget->system_message = widget->chat = widget->ping = widget->command = nullptr;
    widget->chat_history = widget->message_log = nullptr;
    widget->message_background = nullptr;
    widget->chat_was_visible = 0;
    widget->base.base.methods = network_session_methods.data();
    widget->base.base.flags |= 1;
    return widget;
}

void __fastcall initialize_network_session(NetworkSessionWidget *widget, void *) {
    gui::widgets::pending::request_general_layout(&widget->base, nullptr);
    widget->mode = network_page_child(widget, "<MODE>", family::text_class_key);
    widget->system_message = network_page_child(widget, "<SYSMSG>", family::text_class_key);
    widget->chat = network_page_child(widget, "<CHAT>", family::edit_text_class_key);
    widget->chat_history = reinterpret_cast<family::ListBox *>(
        network_page_child(widget, "<CHAT_MSG_LIST>", family::list_box_class_key));
    widget->ping = static_cast<Widget *>(pending::find_child(widget, "<PINGDISPLAY>"));
    widget->command = network_page_child(widget, "<MSG>", family::edit_text_class_key);
    widget->message_log = reinterpret_cast<family::ListBox *>(
        network_page_child(widget, "<MSG_LIST>", family::list_box_class_key));
    widget->message_background =
        network_page_child(widget, "<MSG_LIST_BKG>", family::image_box_class_key);
    auto *engine = gui::active_engine();
    if (widget->chat && widget->chat_history && engine) {
        Dword font;
        {
            pending::LocalFrontendString name(region::get() == 6 ? "chat" : "Pchyron");
            font = invoke<Dword>(engine, 7, &name.value);
        }
        if (font) {
            invoke(engine, 14, font);
            const int height = invoke<int>(engine, 17);
            auto *chat = widget->chat;
            chat->bounds.height = height;
            if (auto *child = child_at(chat, 0, 0))
                invoke(child, 6, Dword(16));
            chat->font_style = font;
            widget->chat_history->base.base.font_style = font;
        }
    }
    if (auto *message_log = widget->message_log) {
        set_enabled(&message_log->base.base, 0);
        pending::set_widget_visible(message_log, 0);
    }
    if (widget->message_background)
        pending::set_widget_visible(widget->message_background, 0);
    if (auto *history = widget->chat_history) {
        set_enabled(&history->base.base, 1);
        pending::set_widget_visible(history, 1);
    }
    auto *empty = &lifecycle::construction::unresolved::shared_string;
    if (auto *chat = widget->chat) {
        invoke(chat, 7, empty);
        set_enabled(chat, 0);
        pending::set_widget_visible(chat, 0);
    }
    if (auto *command = widget->command) {
        invoke(command, 7, empty);
        set_enabled(command, 0);
        pending::set_widget_visible(command, 0);
        pending::set_widget_callback(command, 62, filter_network_console_key);
    }
    if (widget->system_message)
        invoke(widget->system_message, 7, empty);
    widget->consumed_messages = 0;
    widget->next_message = 0;
    widget->chat_expiry = 0;
    widget->mode_expiry = 0;
    game::runtime::reset_complete = 0;
}
static ClassRecord network_session_class;
static CallbackRecord network_edit_callback;

static void unregister_network_edit() {
    unregister_callback(&network_edit_callback);
}

static void register_network_edit() {
    register_callback(&network_edit_callback, family::edit_text_class_key, "GEVENT_BEGIN", 18,
                      "NetgameFE::OnEditText", network_edit_event, 0);
    std::atexit(unregister_network_edit);
}

static void unregister_network_session() {
    unregister_class(&network_session_class);
}

static void register_network_session() {
    register_class(&network_session_class, "NetgameFE", create_network_session, "GWidget");
    std::atexit(unregister_network_session);
}
} // namespace nfs::application::frontend
// Register the callback before registering the class.
NFS_KERNEL_INITIALIZER(startup_register_network_edit, ".CRT$XCU_NFS_00690270",
                       nfs::application::frontend::register_network_edit);
NFS_KERNEL_INITIALIZER(startup_register_network_session, ".CRT$XCU_NFS_00690274",
                       nfs::application::frontend::register_network_session);
