#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "controller_interfaces.hpp"
#include <cstring>
namespace nfs::controllers {

ControlValue *construct_control(ControlValue *value) {
    value->index = 0;
    value->value = 0;
    value->methods = value_base_methods;
    value->reference = 0;
    return value;
}

AxisValue *construct_axis(AxisValue *value) {
    value->index = 0;
    value->polarity = 0;
    value->reference = 0;
    value->minimum = 0;
    value->maximum = 0;
    value->methods = axis_value_methods;
    return value;
}

ControlValue *construct_button(ControlValue *value) {
    value->index = 0;
    value->value = 0;
    value->reference = 0;
    value->methods = button_value_methods;
    return value;
}

void set_control_reference(ControlValue *value, Dword reference) {
    value->reference = reference;
}

void set_key(ControlValue *value, Dword code, Dword reference) {
    value->index = code;
    value->reference = reference;
}

void set_axis(AxisValue *value, Dword index, Dword reference, Dword minimum, Dword maximum,
              Dword polarity) {
    value->index = index;
    value->reference = reference;
    value->minimum = minimum;
    value->maximum = maximum;
    value->polarity = polarity;
}

void set_pov(ControlValue *value, Dword index, Dword reference, Dword direction) {
    value->index = index;
    value->reference = reference;
    value->value = direction;
}

Controller *construct_controller(Controller *owner, int device, const char *name) {
    owner->device = device;
    owner->methods = controller_methods;
    owner->active = 0;
    owner->name[0] = 0;
    if (name)
        std::strncpy(owner->name, name, 64);
    else {
        owner->name[0] = 0;
        owner->active = 0;
    }
    return owner;
}

AxisList *construct_axis_list(AxisList *owner) {
    owner->methods = list_base_methods;
    owner->count = 0;
    for (auto &value : owner->values)
        construct_axis(&value);
    owner->methods = axis_list_methods;
    return owner;
}

ButtonList *construct_button_list(ButtonList *owner) {
    owner->methods = list_base_methods;
    owner->count = 0;
    for (auto &value : owner->values)
        construct_button(&value);
    owner->methods = button_list_methods;
    return owner;
}

PovList *construct_pov_list(PovList *owner) {
    owner->methods = list_base_methods;
    owner->count = 0;
    for (auto &value : owner->values) {
        construct_control(&value);
        value.methods = pov_value_methods;
    }
    owner->methods = pov_list_methods;
    return owner;
}

void destroy_axis_list(void *owner) {
    at<const Method *>(owner, 0) = axis_list_methods;
    at<const Method *>(owner, 0) = list_base_methods;
}

void destroy_button_list(void *owner) {
    at<const Method *>(owner, 0) = button_list_methods;
    at<const Method *>(owner, 0) = list_base_methods;
}

void destroy_pov_list(void *owner) {
    at<const Method *>(owner, 0) = pov_list_methods;
    at<const Method *>(owner, 0) = list_base_methods;
}

void destroy_joystick(Joystick *owner) {
    owner->base.methods = joystick_methods;
    destroy_pov_list(&owner->povs);
    destroy_button_list(&owner->buttons);
    destroy_axis_list(&owner->axes);
    owner->base.methods = controller_methods;
}

void destroy_keyboard(void *owner) {
    at<const Method *>(owner, 0) = controller_methods;
}

void destroy_key_list(void *owner) {
    at<const Method *>(owner, 0) = list_base_methods;
}
KeyboardValues keyboard_values{};
// The native startup constructs124 values and registers no finalizer.
static void initialize_keyboard_values() {
    for (auto &value : keyboard_values.values) {
        construct_control(&value);
        value.methods = key_value_methods;
    }
}
NFS_KERNEL_INITIALIZER(startup_initialize_keyboard_values, ".CRT$XCU_NFS_006902AC",
                       initialize_keyboard_values);

void *keyboard_controller() {
    if (!primary) {
        auto *owner = static_cast<Keyboard *>(
            lifecycle::allocate(80, "Controllers::Keyboard instance", lifecycle::ui_pool));
        if (!owner) {
            primary = nullptr;
            return primary;
        }
        kernel::lifecycle::Cleanup allocation([&] {
            game::runtime::track::pending::release_named(owner, "Controllers::Keyboard instance", lifecycle::ui_pool);
        });
        construct_controller(&owner->base, -1, "keyboard");
        kernel::lifecycle::Cleanup controller([&] { destroy_keyboard(owner); });
        if (!secondary) {
            auto *keys = static_cast<Method const **>(
                lifecycle::allocate(4, "Controllers::Keylist instance", lifecycle::ui_pool));
            if (keys) {
                *keys = list_base_methods;
                *keys = key_list_methods;
            }
            secondary = keys;
            for (Dword index = 0; index < invoke<Dword>(keys, 7) && keyboard_names[index].code;
                 ++index)
                set_key(&keyboard_values.values[index], keyboard_names[index].code,
                        keyboard_names[index].reference);
        }
        owner->keys = secondary;
        owner->base.methods = keyboard_methods;
        controller.dismiss();
        allocation.dismiss();
        primary = owner;
    }
    return primary;
}
// Capacity check precedes population of the saved counts.
AxisValue *axis_slot(Joystick *owner, Dword index) {
    return index < 24 ? &owner->axes.values[index] : nullptr;
}

ControlValue *button_slot(Joystick *owner, Dword index) {
    return index < 24 ? &owner->buttons.values[index] : nullptr;
}

ControlValue *pov_slot(Joystick *owner, Dword index) {
    return index < 4 ? &owner->povs.values[index] : nullptr;
}
} // namespace nfs::controllers
