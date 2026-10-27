#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
#include <cstring>
#include "nfs/core/lifecycle/subsystem_runtime.hpp"

namespace nfs::services {

void pending::set_axis_binding(void *object, Dword axis, Dword sign, Dword device) {
    auto &value = field<Dword>(object, 0);
    field<Byte>(object, 0) = 3;
    value =
        (((axis << 8) ^ value) & 16776960u ^ value) & 16777215u ^ (sign & 15) << 24 ^ device << 28;
}

void pending::set_button_binding(void *object, Byte button, Dword device) {
    field<Byte>(object, 0) = 4;
    field<Byte>(object, 1) = button;
    field<std::uint16_t>(object, 2) = static_cast<std::uint16_t>(device);
}
 
void pending::reset_input_device(void *, Dword) {
}

void set_input_binding(void *object, int index, Dword binding) {
    auto *destination = static_cast<Byte *>(object) + 36 + Dword(index) * 4;
    switch (static_cast<Byte>(binding)) {
    case 0:
        field<Dword>(destination, 0) = 0;
        return;
    case 1:
        field<Byte>(destination, 0) = 1;
        field<Byte>(destination, 2) = Byte(binding >> 16);
        field<Byte>(destination, 1) = Byte(binding >> 8);
        field<Dword>(destination, 0) = (field<Dword>(destination, 0) & 16777215u) ^
                                      ((binding >> 24) & 15) << 24 ^ (binding & 4026531840u);
        break;
    case 2:
        field<Byte>(destination, 0) = 2;
        field<Dword>(destination, 0) = (field<Dword>(destination, 0) & 255u) ^
                                      ((binding >> 8) & 1048575u) << 8 ^ (binding & 4026531840u);
        break;
    case 3:
        pending::set_axis_binding(destination, (binding >> 8) & 65535, (binding >> 24) & 15,
                                  binding >> 28);
        break;
    case 4:
        pending::set_button_binding(destination, Byte(binding >> 8), binding >> 16);
        return;
    }
    pending::reset_input_device(object, binding >> 28);
}

void initialize_input_bindings(void *object) {
    void *rules = lifecycle::manager->slots[72 / 4];
    const unsigned source[] = {2548, 2544, 2552, 2556, 2560, 2564, 2572, 2568, 2584,
                               2576, 2580, 2588, 2592, 2596, 2600, 2604, 2608, 2612,
                               2616, 2620, 2624, 2628, 2632, 2636, 2640, 2644};
    const unsigned target[] = {0,  1,  2,  3,  4,  5,  7,  6,  10, 8,  9,  11, 12,
                               13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25};
    for (unsigned player = 0; player < 2; ++player)
        for (unsigned index = 0; index < 26; ++index)
            set_input_binding(object, int(player * 26 + target[index]),
                              field<Dword>(rules, source[index]));
}
// and  are confirmed single-RET callees.
void reset_input_state(void *object) {
    std::memset(static_cast<Byte *>(object) + 36, 0, 208);
    initialize_input_bindings(object);
    for (int index = 0; index < 256; ++index) {
        const int scaled = index * 50;
        const Byte value = scaled < 2501 ? 0 : Byte(scaled / 255 + index - 50);
        field<Byte>(object, 500 + index) = value;
        field<Byte>(object, 756 + index) = value;
    }
}
} // namespace nfs::services
