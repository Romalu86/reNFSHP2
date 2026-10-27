#include "controller_interfaces.hpp"
#include "nfs/core/errors/assertions.hpp"
namespace nfs::controllers {
// Default controller methods return zero or the receiver.

Dword __fastcall value_axis_view(void *, void *) {
    return 0;
}

Dword __fastcall value_axis_edit_view(void *, void *) {
    return 0;
}

Dword __fastcall value_button_view(void *, void *) {
    return 0;
}

Dword __fastcall value_button_edit_view(void *, void *) {
    return 0;
}

Dword __fastcall value_pov_view(void *, void *) {
    return 0;
}

Dword __fastcall value_pov_edit_view(void *, void *) {
    return 0;
}

void *__fastcall key_view(void *value, void *) {
    return value;
}

void *__fastcall key_edit_view(void *value, void *) {
    return value;
}

Dword __fastcall value_key_view(void *, void *) {
    return 0;
}

Dword __fastcall value_key_edit_view(void *, void *) {
    return 0;
}

void *__fastcall pov_view(void *value, void *) {
    return value;
}

void *__fastcall pov_edit_view(void *value, void *) {
    return value;
}

void *__fastcall axis_view(void *value, void *) {
    return value;
}

void *__fastcall axis_edit_view(void *value, void *) {
    return value;
}

void *__fastcall button_view(void *value, void *) {
    return value;
}

void *__fastcall button_edit_view(void *value, void *) {
    return value;
}

Dword __fastcall key_matches(ControlValue *value, void *, const void *binding) {
    return at<Byte>(binding, 0) == 4 && at<Byte>(binding, 1) == value->index;
}

Dword __fastcall key_matches_exact(ControlValue *value, void *, const void *binding) {
    return at<Byte>(binding, 0) == 4 && at<Byte>(binding, 1) == value->index;
}

Dword __fastcall key_matches_value(ControlValue *value, void *, const void *binding, Dword) {
    return at<Byte>(binding, 0) == 4 && at<Byte>(binding, 1) == value->index;
}

Dword __fastcall axis_matches(AxisValue *value, void *, const void *binding) {
    return at<Byte>(binding, 0) == 1 && Dword(at<Byte>(binding, 3) & 15) == value->index &&
           at<Byte>(binding, 2) == value->minimum && at<Byte>(binding, 1) == value->maximum;
}
// Only AL is defined on the early mismatch paths.
Byte __fastcall axis_matches_exact(AxisValue *value, void *, const void *binding) {
    return Byte(axis_matches(value, nullptr, binding));
}
// Only AL is defined for equal-endpoint and type-mismatch paths.
Byte __fastcall axis_matches_value(AxisValue *value, void *, const void *binding, int current) {
    if (at<Byte>(binding, 0) != 1 || Dword(at<Byte>(binding, 3) & 15) != value->index)
        return 0;
    const int first = int(value->minimum), second = int(value->maximum);
    if (first == second)
        return current == first;
    return first < second ? first <= current && current <= second
                          : second <= current && current <= first;
}

Dword __fastcall pov_matches(ControlValue *value, void *, const void *binding) {
    return at<Byte>(binding, 0) == 3 && Dword(at<Byte>(binding, 3) & 15) == value->index;
}
namespace {
Dword direction_from_value(int direction) {
    if (direction == -1)
        return 0;
    if (!direction)
        return 1;
    if (direction == 18000)
        return 2;
    if (direction == 27000)
        return 3;
    return direction == 9000 ? 4 : 0;
}
} // namespace

Dword __fastcall pov_matches_exact(ControlValue *value, void *, const void *binding) {
    const Dword direction = direction_from_value(at<std::uint16_t>(binding, 1));
    return at<Byte>(binding, 0) == 3 && Dword(at<Byte>(binding, 3) & 15) == value->index &&
           direction == value->value;
}

Dword __fastcall pov_matches_value(ControlValue *value, void *, const void *binding, int current) {
    const Dword direction = direction_from_value(current);
    return at<Byte>(binding, 0) == 3 && Dword(at<Byte>(binding, 3) & 15) == value->index &&
           direction == value->value;
}

Dword __fastcall button_matches(ControlValue *value, void *, const void *binding) {
    return at<Byte>(binding, 0) == 2 && ((at<Dword>(binding, 0) >> 8) & 1048575) == value->index;
}

Dword __fastcall button_matches_exact(ControlValue *value, void *, const void *binding) {
    return at<Byte>(binding, 0) == 2 && ((at<Dword>(binding, 0) >> 8) & 1048575) == value->index;
}
// The second argument is discarded by RET8.
Dword __fastcall button_matches_value(ControlValue *value, void *, const void *binding, Dword) {
    return at<Byte>(binding, 0) == 2 && ((at<Dword>(binding, 0) >> 8) & 1048575) == value->index;
}
#define METHOD(value) reinterpret_cast<Method>(value)
const Method value_base_methods[11] = {METHOD(assertions::pure_virtual_call),
                                       METHOD(assertions::pure_virtual_call),
                                       METHOD(assertions::pure_virtual_call),
                                       METHOD(value_axis_edit_view),
                                       METHOD(value_axis_view),
                                       METHOD(value_button_edit_view),
                                       METHOD(value_button_view),
                                       METHOD(value_pov_edit_view),
                                       METHOD(value_pov_view),
                                       METHOD(value_key_edit_view),
                                       METHOD(value_key_view)};
const Method key_value_methods[11] = {METHOD(key_matches),       METHOD(key_matches_exact),
                                      METHOD(key_matches_value), METHOD(value_axis_edit_view),
                                      METHOD(value_axis_view),   METHOD(value_button_edit_view),
                                      METHOD(value_button_view), METHOD(value_pov_edit_view),
                                      METHOD(value_pov_view),    METHOD(key_edit_view),
                                      METHOD(key_view)};
const Method axis_value_methods[11] = {
    METHOD(axis_matches),        METHOD(axis_matches_exact),  METHOD(axis_matches_value),
    METHOD(axis_edit_view),      METHOD(axis_view),           METHOD(value_button_edit_view),
    METHOD(value_button_view),   METHOD(value_pov_edit_view), METHOD(value_pov_view),
    METHOD(value_key_edit_view), METHOD(value_key_view)};
const Method button_value_methods[11] = {
    METHOD(button_matches),       METHOD(button_matches_exact), METHOD(button_matches_value),
    METHOD(value_axis_edit_view), METHOD(value_axis_view),      METHOD(button_edit_view),
    METHOD(button_view),          METHOD(value_pov_edit_view),  METHOD(value_pov_view),
    METHOD(value_key_edit_view),  METHOD(value_key_view)};
const Method pov_value_methods[11] = {
    METHOD(pov_matches),          METHOD(pov_matches_exact), METHOD(pov_matches_value),
    METHOD(value_axis_edit_view), METHOD(value_axis_view),   METHOD(value_button_edit_view),
    METHOD(value_button_view),    METHOD(pov_edit_view),     METHOD(pov_view),
    METHOD(value_key_edit_view),  METHOD(value_key_view)};
#undef METHOD
} // namespace nfs::controllers
