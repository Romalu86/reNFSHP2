#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../profiles/ui/frontend_profile_controls.hpp"
#include "../../gui/lifecycle/gui_named_identifiers.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
void *create_controller_page(void *);
void *create_deadzone(void *);
void *create_control_remap(void *);
static ClassRecord controller_classes[3];

static void unregister_controller_classes() {
    for (unsigned index = 3; index; --index)
        unregister_class(controller_classes + index - 1);
}

static void register_controller_classes() {
    register_class(controller_classes, "ControllerScreen", create_controller_page, "GWidget");
    register_class(controller_classes + 1, "HP2ControlRemapDlg", create_control_remap, "GMsgBox");
    register_class(controller_classes + 2, "DeadZoneDialog", create_deadzone, "GDialog");
    std::atexit(unregister_controller_classes);
}
// Only the low return byte is consumed by callers.
static std::uint8_t reset_default_controller_bindings() {
    controllers::get_bindings();
    controllers::get_definitions();
    controllers::load_default_maps(controllers::get_default_maps());
    void *maps = controllers::get_default_maps();
    void *rules = system(72);
    int selected = -1;
    const char *name = "keyboard";
    if (void *entry = controllers::controller_at(controllers::get_bindings(), 0)) {
        void *definition = at<void *>(entry, 4);
        if (!definition)
            return 0;
        selected = at<int>(entry, 0);
        name = reinterpret_cast<const char *>(bytes(definition) + 8);
    }
    void *map = controllers::find_default_map(maps, name);
    if (!map)
        return 0;
    controllers::select_primary_controller(controllers::get_bindings(), selected);
    void *definition = at<void *>(map, 0);
    void *axes = invoke<void *>(definition, 6), *povs = invoke<void *>(definition, 8),
         *buttons = invoke<void *>(definition, 7);
    invoke(definition, 5);
    for (unsigned index = 0; index < 48; ++index) {
        const auto *entry = bytes(map) + 4 + 12 * index;
        const Dword action = at<Dword>(entry, 0), kind = at<Dword>(entry, 4),
                   element = at<Dword>(entry, 8);
        controllers::action_name(int(action));
        void *binding = bytes(rules) + 2544 + 4 * action;
        if (kind == 2) {
            at<std::uint8_t>(binding, 0) = 4;
            at<std::uint8_t>(binding, 1) = std::uint8_t(element);
        } else if (kind == 1) {
            const void *axis = bytes(axes) + 8 + 24 * element;
            const Dword group = at<Dword>(axis, 4), minimum = at<Dword>(axis, 16),
                       maximum = at<Dword>(axis, 20);
            at<std::uint8_t>(binding, 0) = 1;
            at<std::uint8_t>(binding, 2) = std::uint8_t(minimum);
            at<std::uint8_t>(binding, 1) = std::uint8_t(maximum);
            at<Dword>(binding, 0) =
                (at<Dword>(binding, 0) & 16777215) ^ ((group & 15) << 24) ^ (Dword(selected) << 28);
        } else if (kind == 4) {
            const void *pov = bytes(povs) + 8 + 16 * element;
            const Dword group = at<Dword>(pov, 4),
                       direction = Dword(controllers::pov_direction(int(at<Dword>(pov, 12))));
            at<std::uint8_t>(binding, 0) = 3;
            at<Dword>(binding, 0) =
                ((group & 15) << 24) ^
                ((((direction & 65535) << 8) ^ (at<Dword>(binding, 0) & 4278190335u)) & 16777215) ^
                (Dword(selected) << 28);
        } else if (kind == 3) {
            const Dword group = at<Dword>(buttons, 12 + 16 * element);
            at<std::uint8_t>(binding, 0) = 2;
            at<Dword>(binding, 0) =
                ((group & 1048575) << 8) ^ (at<Dword>(binding, 0) & 255) ^ (Dword(selected) << 28);
        }
    }
    return 1;
}
static bool confirm_controller_change(Dword reference) {
    pending::MessageBox box;
    {
        pending::LocalFrontendString text;
        gui::string_reference(&text.value, reference);
        pending::construct_message_box(&box, &text.value, 66);
    }
    int result;
    try {
        result = pending::run_message_box(&box, nullptr, 0);
    } catch (...) {
        pending::destroy_message_box(&box);
        throw;
    }
    pending::destroy_message_box(&box);
    return result == 2;
}

static Dword __cdecl cancel_controller_changes(void *sender, Dword event, Dword, Dword) {
    if (event != 43)
        return 0;
    auto *parent = static_cast<Widget *>(sender)->parent;
    if (!parent || !invoke<Dword>(parent, 2, controller_page_class_key))
        return 0;
    auto *page = reinterpret_cast<ControllerPage *>(parent);
    if (!std::memcmp(page->bindings, bytes(page->preferences) + 56, sizeof(page->bindings)))
        return 0;
    if (confirm_controller_change(1811)) {
        if (page->preferences)
            std::memcpy(page->bindings, bytes(page->preferences) + 56, sizeof(page->bindings));
        refresh_controller_bindings(page);
    }
    return 1;
}

static Dword __cdecl reset_controller_defaults(void *sender, Dword event, Dword, Dword) {
    if (event != 43)
        return 0;
    auto *parent = static_cast<Widget *>(sender)->parent;
    if (!parent || !invoke<Dword>(parent, 2, controller_page_class_key))
        return 0;
    auto *page = reinterpret_cast<ControllerPage *>(parent);
    if (confirm_controller_change(1812)) {
        reset_default_controller_bindings();
        if (page->preferences)
            std::memcpy(page->bindings, bytes(page->preferences) + 56, sizeof(page->bindings));
        refresh_controller_bindings(page);
    }
    return 1;
}
static CallbackRecord controller_callbacks[2];

static void unregister_controller_callbacks() {
    for (unsigned index = 2; index; --index)
        unregister_callback(controller_callbacks + index - 1);
}

static void register_controller_callbacks() {
    register_callback(controller_callbacks, family::button_class_key, "BEVENT_PRESSED", 43,
                      "ControllerScreen::CancelChanges", cancel_controller_changes, 0);
    register_callback(controller_callbacks + 1, family::button_class_key, "BEVENT_PRESSED", 43,
                      "ControllerScreen::ResetToDefaults", reset_controller_defaults, 0);
    std::atexit(unregister_controller_callbacks);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_initialize_modify_control, ".CRT$XCU_NFS_00690068",
                       gui::widgets::identifiers::initialize_modify_control);
NFS_KERNEL_INITIALIZER(startup_register_controller_classes, ".CRT$XCU_NFS_0069006C",
                       register_controller_classes);
NFS_KERNEL_INITIALIZER(startup_register_controller_callbacks, ".CRT$XCU_NFS_00690070",
                       register_controller_callbacks);
} // namespace
} // namespace nfs::application::frontend
