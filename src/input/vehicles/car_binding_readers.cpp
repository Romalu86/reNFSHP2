#include "car_binding_readers.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/network/session/network_runtime.hpp"
namespace nfs::game::runtime::ai::cars {

static void *find_axis_deadzone(void *records, int axis, int minimum, int maximum) {
    for (unsigned index = 0; index < 32; ++index) {
        void *record = bytes(records) + index * 16;
        if (at<int>(record, 0) == axis && at<int>(record, 4) == minimum &&
            at<int>(record, 8) == maximum)
            return record;
    }
    return nullptr;
}
// The saved type-zero reader writes its flag and returns zero.
static int __cdecl read_empty_binding(Dword, Byte *analog) {
    *analog = 0;
    return 0;
}

static int __cdecl read_axis_binding(Dword binding, Byte *analog) {
    *analog = 1;
    void *owner = system(100);
    const Dword device_index = binding >> 28;
    int value = 0;
    if (device_index >= 32 || !at<Byte>(owner, 1012 + device_index * 8))
        return 0;
    void *device = at<void *>(owner, 1016 + device_index * 8);
    if (!device)
        return 0;
    const Dword axis = (binding >> 24) & 15;
    const int current = at<int>(device, 4 + axis * 4) >> 8;
    int maximum = (binding >> 8) & 255;
    int minimum = (binding >> 16) & 255;
    void *record = find_axis_deadzone(bytes(system(72)) + 52060, axis, minimum, maximum);
    const int deadzone = record ? at<int>(record, 12) : 13;
    record = find_axis_deadzone(bytes(system(72)) + 52060, axis, minimum, maximum);
    if (current == maximum)
        value = 255;
    else {
        if (current == minimum)
            return 0;
        Dword numerator = 0, denominator = 0;
        bool within = true;
        if (maximum > minimum) {
            if (current < minimum || current > maximum)
                within = false;
            else {
                numerator = Dword(current - minimum);
                denominator = Dword(maximum - minimum);
            }
        } else if (maximum < minimum) {
            if (current < maximum || current > minimum)
                within = false;
            else {
                numerator = Dword(minimum - current);
                denominator = Dword(minimum - maximum);
            }
        }
        if (within) {
            const long double scaled =
                (static_cast<long double>(255.0) / int(std::uint16_t(denominator))) *
                int(std::uint16_t(numerator));
            value = std::int32_t(Dword(hardware::truncate_integer_native80(&scaled)));
        }
        if (record && at<int>(record, 12) > 0 && value < at<int>(record, 12))
            return 0;
        if (value <= 0)
            return value;
    }
    if (deadzone > 0 && value <= deadzone)
        return 0;
    return value;
}

static int __cdecl read_button_binding(Dword binding, Byte *analog) {
    *analog = 0;
    const Dword device_index = binding >> 28;
    const Dword button = (binding >> 8) & 1048575;
    void *device = nullptr;
    if (device_index < 32 && at<Byte>(system(100), 1012 + device_index * 8)) {
        device = at<void *>(system(100), 1016 + device_index * 8);
        if (device && at<Byte>(device, 52 + button))
            return 255;
    }
    switch (button) {
    case 32:
        return at<Byte>(device, 36) == 0 ? 255 : 0;
    case 33:
        return at<Byte>(device, 36) == 40 ? 255 : 0;
    case 34:
        return at<Byte>(device, 36) == 80 ? 255 : 0;
    case 35:
        return at<Byte>(device, 36) == 120 ? 255 : 0;
    default:
        return 0;
    }
}
// An unavailable device returns FFFFh, as in the branch.
static int __cdecl read_pov_binding(Dword binding, Byte *analog) {
    void *owner = system(100);
    *analog = 0;
    const Dword index = binding >> 28;
    if (index < 32 && at<Byte>(owner, 1012 + index * 8)) {
        void *device = at<void *>(owner, 1016 + index * 8);
        if (device)
            return at<Dword>(device, 36 + ((binding >> 24) & 15) * 4) == ((binding >> 8) & 65535)
                       ? 255
                       : 0;
    }
    return 65535;
}
// The AND EAX,FFh returns 255 for a held key.
static int __cdecl read_keyboard_binding(Dword binding, Byte *analog) {
    *analog = 0;
    if (network::session_active() && (phase_reset || reset_complete || reset_requested))
        return 0;
    const Dword key = (binding >> 8) & 255;
    return keyboard_snapshot && at<Byte>(reinterpret_cast<void *>(keyboard_snapshot), key) ? 255
                                                                                           : 0;
}
const BindingReader binding_readers[5] = {read_empty_binding, read_axis_binding,
                                          read_button_binding, read_pov_binding,
                                          read_keyboard_binding};

void read_cop_action_bindings(void *owner, Dword player, Dword *controls) {
    Dword held = 0;
    const Dword offset = (player ? 26 : 0) * 4;
    if (network::session_active() && (phase_reset || reset_complete || reset_requested)) {
        at<Dword>(owner, 4 + player * 4) = 0;
        controls[0] = 0;
        return;
    }
    // Both calls use the high byte of their already initialized player argument.
    Dword reader_scratch = player;
    auto *analog = reinterpret_cast<Byte *>(&reader_scratch) + 3;
    std::int8_t type = at<std::int8_t>(owner, 80 + offset);
    if (type && binding_readers[type](at<Dword>(owner, 80 + offset), analog))
        held = 64;
    type = at<std::int8_t>(owner, 64 + offset);
    if (type && binding_readers[type](at<Dword>(owner, 64 + offset), analog))
        held |= 4;
    const Dword previous = at<Dword>(owner, 4 + player * 4);
    at<Dword>(owner, 4 + player * 4) = held & 16718908;
    controls[0] = ~previous & held;
}
} // namespace nfs::game::runtime::ai::cars
