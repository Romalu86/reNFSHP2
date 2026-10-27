#include "../../profiles/ui/frontend_profile_controls.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/resources/localization/region.hpp"
#include <cstring>
namespace nfs::application::frontend {
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;

static void *find_controller_control(void *controller, const Dword *binding) {
    unsigned slot;
    switch (std::uint8_t(*binding)) {
    case 4:
        slot = 5;
        break;
    case 1:
        slot = 6;
        break;
    case 2:
        slot = 7;
        break;
    case 3:
        slot = 8;
        break;
    default:
        return nullptr;
    }
    void *list = invoke<void *>(controller, slot);
    return list ? invoke<void *>(list, 6, binding) : nullptr;
}

static void *find_controller_control_value(void *controller, const Dword *binding, Dword value) {
    unsigned slot;
    switch (std::uint8_t(*binding)) {
    case 4:
        slot = 5;
        break;
    case 1:
        slot = 6;
        break;
    case 2:
        slot = 7;
        break;
    case 3:
        slot = 8;
        break;
    default:
        return nullptr;
    }
    void *list = invoke<void *>(controller, slot);
    return list ? invoke<void *>(list, 5, binding, value) : nullptr;
}
static void *binding_controller(void *owner, const Dword *binding) {
    const auto kind = std::uint8_t(*binding);
    if (kind == 4)
        return controllers::keyboard_controller();
    const Dword device = kind >= 1 && kind <= 3 ? *binding >> 28 : ~Dword(0);
    for (Dword index = 0; index < at<Dword>(owner, 0); ++index)
        if (device == at<Dword>(owner, 4 + index * 8))
            return at<void *>(owner, 8 + index * 8);
    return nullptr;
}

static void *find_list_control(void *owner, const Dword *binding) {
    if (std::uint8_t(*binding) == 4)
        return find_controller_control(controllers::keyboard_controller(), binding);
    void *controller = binding_controller(owner, binding);
    return controller ? find_controller_control(controller, binding) : nullptr;
}

static void *find_list_control_value(void *owner, const Dword *binding, Dword value) {
    if (std::uint8_t(*binding) == 4)
        return find_controller_control_value(controllers::keyboard_controller(), binding, value);
    void *controller = binding_controller(owner, binding);
    return controller ? find_controller_control_value(controller, binding, value) : nullptr;
}
void *resolve_menu_control(void *owner, const Dword *binding, Dword value, bool by_value) {
    return by_value ? find_list_control_value(owner, binding, value)
                    : find_list_control(owner, binding);
}

bool bindings_equal(const Dword *left, const Dword *right) {
    const auto kind = std::uint8_t(*left);
    if (!kind && !std::uint8_t(*right))
        return true;
    if (kind != std::uint8_t(*right))
        return false;
    if (kind >= 1 && kind <= 3)
        return *left == *right;
    return kind == 4 && std::uint8_t(*left >> 8) == std::uint8_t(*right >> 8);
}
static void copy_binding(Dword *destination, const Dword *source) {
    if (destination == source)
        return;
    switch (std::uint8_t(*source)) {
    case 0:
        *destination = 0;
        break;
    case 1:
    case 2:
    case 3:
        *destination = *source;
        break;
    case 4:
        services::pending::set_button_binding(destination, std::uint8_t(*source >> 8),
                                              *source >> 16);
        break;
    }
}

bool assign_controller_binding(ControllerPage *page, Dword action, Dword *binding, Dword value) {
    if (!std::uint8_t(*binding) ||
        (std::uint8_t(*binding) == 4 && std::uint8_t(*binding >> 8) == 1))
        return false;
    void *control = resolve_menu_control(controllers::get_bindings(), binding, value, true);
    if (!control)
        return false;
    if (std::uint8_t(*binding) == 1)
        if (void *axis = invoke<void *>(control, 3)) {
            auto *encoded = reinterpret_cast<std::uint8_t *>(binding);
            encoded[2] = std::uint8_t(at<Dword>(axis, 16));
            encoded[1] = std::uint8_t(at<Dword>(axis, 20));
        }
    Dword *destination = page->bindings + action;
    int duplicate = -1;
    for (int index = 0; index < 18; ++index)
        if (bindings_equal(page->bindings + index, binding)) {
            duplicate = index;
            break;
        }
    const Dword previous = *destination;
    copy_binding(destination, binding);
    if (duplicate != -1)
        copy_binding(page->bindings + duplicate, &previous);
    return duplicate != -1;
}

Dword describe_menu_binding(void *control, gui::String *text) {
    if (!control)
        return 0;
    gui::string_reference(text, at<Dword>(control, 8));
    void *key = invoke<void *>(control, 9);
    if (!key)
        return 1;
    const int scan = at<int>(key, 4);
    if (region::get() != 6) {
        char name[256]{};
        const int length = GetKeyNameTextA(LONG(Dword(scan) << 16), name, 256);
        if (length) {
            const char *replacement = nullptr;
            if (length == 1) {
                if (std::uint8_t(name[0]) == 229)
                    replacement = "\xc5";
                else if (std::uint8_t(name[0]) == 228)
                    replacement = "\xc4";
                else if (std::uint8_t(name[0]) == 246)
                    replacement = "\xd6";
            }
            if (replacement)
                gui::string_assign(text, replacement);
            else if (scan == 69)
                gui::string_reference(text, 1874);
            else
                gui::string_assign(text, name);
        }
    }
    if (scan == 69)
        gui::string_reference(text, 1874);
    return 1;
}
} // namespace nfs::application::frontend
