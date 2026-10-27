#include "controller_interfaces.hpp"
#include "nfs/core/errors/assertions.hpp"
namespace nfs::controllers {
// Complete XOR AL,AL;RET leaves.
static Byte __fastcall list_has_axes(void *, void *) {
    return 0;
}

static Byte __fastcall list_has_keys(void *, void *) {
    return 0;
}

static Byte __fastcall list_has_buttons(void *, void *) {
    return 0;
}

static Byte __fastcall list_has_povs(void *, void *) {
    return 0;
}

static Byte __fastcall key_list_supports_keys(void *, void *) {
    return 1;
}

static Byte __fastcall axis_list_supports_axes(void *, void *) {
    return 1;
}

static Byte __fastcall button_list_supports_buttons(void *, void *) {
    return 1;
}

static Byte __fastcall pov_list_supports_povs(void *, void *) {
    return 1;
}

static void *__fastcall delete_list_base(void *owner, void *, unsigned flags) {
    at<const Method *>(owner, 0) = list_base_methods;
    if (flags & 1)
        gui::application_release(owner);
    return owner;
}

static void *__fastcall delete_key_list(void *owner, void *, unsigned flags) {
    destroy_key_list(owner);
    if (flags & 1)
        gui::application_release(owner);
    return owner;
}

static void *__fastcall delete_axis_list(void *owner, void *, unsigned flags) {
    destroy_axis_list(owner);
    if (flags & 1)
        gui::application_release(owner);
    return owner;
}

static void *__fastcall delete_button_list(void *owner, void *, unsigned flags) {
    destroy_button_list(owner);
    if (flags & 1)
        gui::application_release(owner);
    return owner;
}

static void *__fastcall delete_pov_list(void *owner, void *, unsigned flags) {
    destroy_pov_list(owner);
    if (flags & 1)
        gui::application_release(owner);
    return owner;
}

static void *__fastcall find_control(void *owner, void *, const void *binding) {
    for (Dword index = 0; index < invoke<Dword>(owner, 7); ++index) {
        void *value = invoke<void *>(owner, 9, index);
        if (invoke<Byte>(value, 1, binding))
            return value;
    }
    return nullptr;
}

static void *__fastcall find_control_value(void *owner, void *, const void *binding, Dword current) {
    for (Dword index = 0; index < invoke<Dword>(owner, 7); ++index) {
        void *value = invoke<void *>(owner, 9, index);
        if (invoke<Byte>(value, 2, binding, current))
            return value;
    }
    return nullptr;
}

static Dword __fastcall key_list_count(void *, void *) {
    return 124;
}

static Dword __fastcall axis_list_count(void *owner, void *) {
    return at<Dword>(owner, 4);
}

static Dword __fastcall button_list_count(void *owner, void *) {
    return at<Dword>(owner, 4);
}

static Dword __fastcall pov_list_count(void *owner, void *) {
    return at<Dword>(owner, 4);
}

static void *__fastcall key_at(void *owner, void *, Dword index) {
    return index < invoke<Dword>(owner, 7) ? &keyboard_values.values[index] : nullptr;
}

static void *__fastcall mutable_key_at(void *owner, void *, Dword index) {
    return index < invoke<Dword>(owner, 7) ? &keyboard_values.values[index] : nullptr;
}

static void *__fastcall axis_at(void *owner, void *, Dword index) {
    return index < invoke<Dword>(owner, 7) ? bytes(owner) + 8 + index * 24 : nullptr;
}

static void *__fastcall mutable_axis_at(void *owner, void *, Dword index) {
    return index < invoke<Dword>(owner, 7) ? bytes(owner) + 8 + index * 24 : nullptr;
}

static void *__fastcall button_at(void *owner, void *, Dword index) {
    return index < invoke<Dword>(owner, 7) ? bytes(owner) + 8 + index * 16 : nullptr;
}

static void *__fastcall mutable_button_at(void *owner, void *, Dword index) {
    return index < invoke<Dword>(owner, 7) ? bytes(owner) + 8 + index * 16 : nullptr;
}

static void *__fastcall pov_at(void *owner, void *, Dword index) {
    return index < invoke<Dword>(owner, 7) ? bytes(owner) + 8 + index * 16 : nullptr;
}

static void *__fastcall mutable_pov_at(void *owner, void *, Dword index) {
    return index < invoke<Dword>(owner, 7) ? bytes(owner) + 8 + index * 16 : nullptr;
}
#define METHOD(value) reinterpret_cast<Method>(value)
const Method list_base_methods[10] = {METHOD(delete_list_base),
                                      METHOD(list_has_axes),
                                      METHOD(list_has_keys),
                                      METHOD(list_has_buttons),
                                      METHOD(list_has_povs),
                                      METHOD(find_control_value),
                                      METHOD(find_control),
                                      METHOD(assertions::pure_virtual_call),
                                      METHOD(assertions::pure_virtual_call),
                                      METHOD(assertions::pure_virtual_call)};
const Method key_list_methods[10] = {METHOD(delete_key_list),        METHOD(list_has_axes),
                                     METHOD(key_list_supports_keys), METHOD(list_has_buttons),
                                     METHOD(list_has_povs),          METHOD(find_control_value),
                                     METHOD(find_control),           METHOD(key_list_count),
                                     METHOD(mutable_key_at),         METHOD(key_at)};
const Method axis_list_methods[10] = {METHOD(delete_axis_list), METHOD(axis_list_supports_axes),
                                      METHOD(list_has_keys),    METHOD(list_has_buttons),
                                      METHOD(list_has_povs),    METHOD(find_control_value),
                                      METHOD(find_control),     METHOD(axis_list_count),
                                      METHOD(mutable_axis_at),  METHOD(axis_at)};
const Method button_list_methods[10] = {
    METHOD(delete_button_list), METHOD(list_has_axes),
    METHOD(list_has_keys),      METHOD(button_list_supports_buttons),
    METHOD(list_has_povs),      METHOD(find_control_value),
    METHOD(find_control),       METHOD(button_list_count),
    METHOD(mutable_button_at),  METHOD(button_at)};
const Method pov_list_methods[10] = {METHOD(delete_pov_list),        METHOD(list_has_axes),
                                     METHOD(list_has_keys),          METHOD(list_has_buttons),
                                     METHOD(pov_list_supports_povs), METHOD(find_control_value),
                                     METHOD(find_control),           METHOD(pov_list_count),
                                     METHOD(mutable_pov_at),         METHOD(pov_at)};
#undef METHOD
} // namespace nfs::controllers
