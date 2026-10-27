#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../screens/frontend_menu_roots.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include "nfs/network/session/network_rules.hpp"
#include <cstdlib>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
using pending::LocalFrontendString;

static Widget *button_interface(void *sender) {
    auto *button = static_cast<Widget *>(sender);
    if (!button || !invoke<Dword>(button, 2, family::button_class_key))
        return nullptr;
    auto *root = pending::widget_root(button);
    return root && invoke<Dword>(root, 2, interface_class_name) ? root : nullptr;
}
// Shared saved inline queue-pop sequence. Keep the queue captured before disposal.
static void pop_screen_command() {
    void *frontend = system(96);
    if (auto *queue = at<gui::List *>(frontend, 48)) {
        if (void *command = gui::events::pop_front(queue))
            gui::destroy(static_cast<gui::Object *>(command));
        if (!gui::events::list_count(queue)) {
            gui::destroy(reinterpret_cast<gui::Object *>(queue));
            at<gui::List *>(frontend, 48) = nullptr;
        }
    }
}

static Dword __cdecl exit_frontend(void *sender, Dword, Dword, Dword) {
    auto *root = button_interface(sender);
    if (!root)
        return 0;
    if (!launch::field<Dword>(8627776)) {
        pending::MessageBox message;
        {
            LocalFrontendString title, text;
            gui::string_reference(&title.value, 559);
            gui::string_reference(&text.value, 743);
            pending::construct_message_box(&message, &text.value, &title.value, 16450);
        }
        int result;
        try {
            result = pending::run_message_box(&message, nullptr, 0);
        } catch (...) {
            pending::destroy_message_box(&message);
            throw;
        }
        pending::destroy_message_box(&message);
        if (result != 2)
            return 0;
    }
    family::finish_modal(root, 6, 0);
    pending::clear_screen_queue(system(96));
    return 0;
}

static Dword __cdecl race_pressed(void *sender, Dword, Dword, Dword) {
    if (auto *root = button_interface(sender))
        family::finish_modal(root, 4, 0);
    return 0;
}

static Dword __cdecl previous_screen_pressed(void *sender, Dword, Dword, Dword) {
    if (auto *root = button_interface(sender)) {
        pop_screen_command();
        family::finish_modal(root, 5, 0);
    }
    return 0;
}
static bool interface_key_event(void *sender, Dword event, Dword modifiers) {
    return event == 6 && !std::uint8_t(modifiers) && sender &&
           invoke<Dword>(sender, 2, interface_class_name);
}

static Dword __cdecl cancel_to_previous(void *sender, Dword event, Dword key, Dword modifiers) {
    if (!interface_key_event(sender, event, modifiers) || key != 1)
        return 0;
    pop_screen_command();
    family::finish_modal(static_cast<Widget *>(sender), 5, 0);
    return 1;
}

static Dword __cdecl ok_cancel_to_previous(void *sender, Dword event, Dword key, Dword modifiers) {
    if (!interface_key_event(sender, event, modifiers) || (key != 1 && key != 28))
        return 0;
    pop_screen_command();
    family::finish_modal(static_cast<Widget *>(sender), 5, 0);
    return 1;
}

static Dword __cdecl single_to_car_select(void *sender, Dword event, Dword key, Dword modifiers) {
    if (event != 6)
        return 0;
    auto *screen = sender && invoke<Dword>(sender, 2, interface_class_name)
                       ? static_cast<Widget *>(sender)
                       : nullptr;
    if (std::uint8_t(modifiers) || !screen)
        return 0;
    if (key == 1) {
        pop_screen_command();
        family::finish_modal(screen, 5, 0);
    } else if (key == 28) {
        LocalFrontendString type("CarSelectScreen"), name("CarSelect");
        pending::enqueue_screen(system(96), name.release(), type.release());
        family::finish_modal(screen, 4, 0);
    } else
        return 0;
    return 1;
}

static Dword __cdecl single_to_track_select(void *sender, Dword event, Dword key, Dword modifiers) {
    if (!interface_key_event(sender, event, modifiers))
        return 0;
    auto *screen = static_cast<Widget *>(sender);
    if (invoke<Dword>(screen, 3, "SingleChallengeScreen"))
        if (void *view = pending::find_child(screen, "<VIEW>"))
            if (invoke<Dword>(view, 2, challenge_view_class_key) && key == 28 &&
                !can_leave_challenge_view(static_cast<ChallengeView *>(view)))
                return 0;
    if (key == 1) {
        pop_screen_command();
        family::finish_modal(screen, 5, 0);
    } else if (key == 28) {
        LocalFrontendString type("TrackSelectScreen"), name("TrackSelect");
        pending::enqueue_screen(system(96), name.release(), type.release());
        family::finish_modal(screen, 4, 0);
    } else
        return 0;
    return 1;
}

static Dword __cdecl tree_to_car_select(void *sender, Dword event, Dword key, Dword modifiers) {
    if (event != 6 || !sender || !invoke<Dword>(sender, 2, tree_screen_class_key) ||
        std::uint8_t(modifiers))
        return 0;
    if (key == 28)
        family::finish_modal(static_cast<Widget *>(sender), 4, 0);
    else if (key == 1) {
        at<Dword>(system(72), 102920) = ~Dword(0);
        pop_screen_command();
        family::finish_modal(static_cast<Widget *>(sender), 5, 0);
    } else
        return 0;
    return 1;
}
// Complete callback: XOR EAX,EAX; RET.
static Dword __cdecl set_build_information(void *, Dword, Dword, Dword) {
    return 0;
}

static Dword __cdecl save_language(void *, Dword, Dword, Dword) {
    network::rules::pending::clear_language();
    network::rules::pending::load_language(at<Dword>(system(72), 2312));
    if (auto *modal = static_cast<Widget *>(gui::focused_widget()))
        family::finish_modal(modal, 16, 0);
    return 0;
}
// Complete callback: XOR EAX,EAX; RET.
static Dword __cdecl disable_without_second_player(void *, Dword, Dword, Dword) {
    return 0;
}

static Dword __cdecl layout_copyright(void *sender, Dword, Dword, Dword) {
    auto *widget = static_cast<Widget *>(sender);
    invoke(widget, 6, Dword(64));
    invoke(widget, 6, Dword(32));
    gui::engine::Point adjustment{gui::engine::point_methods, 20, 0};
    widget->bounds.left = int(Dword(widget->bounds.left) + Dword(adjustment.x));
    gui::engine::destroy_point(&adjustment);
    return 0;
}
static CallbackRecord common_callbacks[12];

static void unregister_common_callbacks() {
    for (unsigned index = 12; index; --index)
        unregister_callback(common_callbacks + index - 1);
}

static void register_common_callbacks() {
    const struct Definition {
        TypeName type;
        const char *event_name;
        Dword event;
        const char *name;
        Callback callback;
    } definitions[] = {
        {base_class_name, "GEVENT_IDLE", 21, "FE_DisableIfNoP2", disable_without_second_player},
        {family::text_class_key, "GEVENT_BEGIN", 18, "FE_LayoutCopyRightMessage",
         layout_copyright},
        {family::button_class_key, "BEVENT_PRESSED", 43, "FE_PreviousScreen",
         previous_screen_pressed},
        {family::button_class_key, "BEVENT_PRESSED", 43, "FE_Race", race_pressed},
        {family::button_class_key, "BEVENT_PRESSED", 43, "FE_Exit", exit_frontend},
        {interface_class_name, "GEVENT_KEYDOWN", 6, "FE_KeyDownCancelToPrevious",
         cancel_to_previous},
        {interface_class_name, "GEVENT_KEYDOWN", 6, "FE_KeyDownOKCancelToPrevious",
         ok_cancel_to_previous},
        {interface_class_name, "GEVENT_KEYDOWN", 6, "FE_KeyDownCancelToPrevious_SingleToCarSelect",
         single_to_car_select},
        {interface_class_name, "GEVENT_KEYDOWN", 6,
         "FE_KeyDownCancelToPrevious_SingleToTrackSelect", single_to_track_select},
        {interface_class_name, "GEVENT_KEYDOWN", 6, "FE_KeyDownCancelToPrevious_TreeToCarSelect",
         tree_to_car_select},
        {family::text_class_key, "GEVENT_INITIALUPDATE", 19, "FE_SetBuildInfo",
         set_build_information},
        {family::button_class_key, "BEVENT_PRESSED", 43, "FE_SaveLanguage", save_language}};
    for (unsigned index = 0; index < 12; ++index) {
        const auto &entry = definitions[index];
        register_callback(common_callbacks + index, entry.type, entry.event_name, entry.event,
                          entry.name, entry.callback, 0);
    }
    std::atexit(unregister_common_callbacks);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_common_callbacks, ".CRT$XCU_NFS_00690080",
                       register_common_callbacks);
} // namespace
} // namespace nfs::application::frontend
