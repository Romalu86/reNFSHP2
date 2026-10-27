#include "car_binding_readers.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/network/session/network_runtime.hpp"

namespace nfs::game::runtime::ai::cars {

Dword throttle_last_active_tick;
Dword throttle_inactive_since_tick;


void read_car_action_bindings(void *input, Dword player, Dword *controls) {
    const Dword bank_offset = (player ? 26u : 0u) * 4u;
    Dword held = 0;
    if (network::session_active() && (phase_reset || reset_complete || reset_requested)) {
        const Dword previous = at<volatile Dword>(input, 4 + player * 4);
        at<Dword>(input, 4 + player * 4) = 0;
        controls[0] = ~previous & held;
        return;
    }

    // Initialize both steering flags to digital mode for bindings that do not set them.
     
     
     
    Byte primary_analog = 0;
    Byte right_analog = 0;
    auto read_binding = [&](unsigned binding_offset, Byte *analog) -> int {
        const std::int8_t type = at<std::int8_t>(input, bank_offset + binding_offset);
        if (!type)
            return 0;
        const Dword binding = at<Dword>(input, bank_offset + binding_offset);
        return binding_readers[type](binding, analog);
    };

    const int left = read_binding(36, &primary_analog);
    const int right = read_binding(40, &right_analog);

    // SUB/CDQ/SUB/SAR: wrapped subtraction, then signed division by2 toward0.
    const Dword difference = Dword(right) - Dword(left);
    const Dword rounded_difference = difference + (difference >> 31);
    const Dword half_difference =
        (rounded_difference >> 1) | (rounded_difference & 2147483648u);
    const Byte steering = Byte(half_difference);
    at<Byte>(controls, 4) = steering;

    if ((primary_analog && right_analog) ||
        (left != 0 && right == 0 && primary_analog) ||
        (left == 0 && right != 0 && right_analog))
        held = 32768;

    auto *rules = static_cast<Dword *>(system(72));
    if (network::packets::rule_value(rules, 6) == 2 ||
        network::packets::rule_value(rules, 6) == 3)
        at<Byte>(controls, 4) = Byte(0u - Dword(steering));

    Byte throttle = Byte(read_binding(44, &primary_analog));
    at<Byte>(controls, 5) = throttle;
    if (!throttle) {
        throttle = Byte(read_binding(48, &primary_analog));
        at<Byte>(controls, 5) = throttle;
    }
    if (primary_analog)
        held |= 8192;

    if (!at<Byte>(controls, 5)) {
        if (!throttle_inactive_since_tick)
            throttle_inactive_since_tick = graphics::unresolved::camera_state;
    } else {
        const Dword tick = graphics::unresolved::camera_state;
        throttle_inactive_since_tick = 0;
        throttle_last_active_tick = tick;
    }

    Byte brake = Byte(read_binding(52, &primary_analog));
    at<Byte>(controls, 6) = brake;
    if (!brake) {
        brake = Byte(read_binding(56, &primary_analog));
        at<Byte>(controls, 6) = brake;
    }
    if (primary_analog)
        held |= 16384;

    // The remaining calls use the primary flag only as an output argument.
    auto pressed = [&](unsigned binding_offset) {
        return read_binding(binding_offset, &primary_analog) != 0;
    };


    if (pressed(60)) // InputHandBrake.
        held |= 2;
    if (pressed(92)) // InputHorn.
        held |= 512;
    if (pressed(80)) // InputLookBehind.
        held |= 64;


    rules = static_cast<Dword *>(system(72));
    if (network::packets::rule_value(rules, 6) == 2 ||
        network::packets::rule_value(rules, 6) == 3) {
        if (pressed(88)) // InputLookRight.
            held |= 128;
        if (pressed(84)) // InputLookLeft.
            held |= 256;
    } else {
        if (pressed(84)) // InputLookLeft.
            held |= 128;
        if (pressed(88)) // InputLookRight.
            held |= 256;
    }

    if (pressed(96)) // InputCallBackUp.
        held |= 1024;
    if (pressed(100)) // InputCallRoadBlock.
        held |= 2048;
    if (pressed(104)) // InputCallCopter.
        held |= 4096;
    if (pressed(64)) // InputCycleCamera.
        held |= 4;
    if (pressed(76)) // InputReset.
        held |= 32;
    if (pressed(68)) // InputShiftUp.
        held |= 8;
    if (pressed(72)) // InputShiftDown.
        held |= 16;
    if (pressed(108)) // InputGear_Reverse.
        held |= 65536;
    if (pressed(112)) // InputGear_Neutral.
        held |= 131072;
    if (pressed(116)) // InputGear_1.
        held |= 262144;
    if (pressed(120)) // InputGear_2.
        held |= 524288;
    if (pressed(124)) // InputGear_3.
        held |= 1048576;
    if (pressed(128)) // InputGear_4.
        held |= 2097152;
    if (pressed(132)) // InputGear_5.
        held |= 4194304;
    if (pressed(136)) // InputGear_6.
        held |= 8388608;

    const Dword previous = at<Dword>(input, 4 + player * 4);
    at<Dword>(input, 4 + player * 4) = held & 16718908;
    controls[0] = ~previous & held;
}

} // namespace nfs::game::runtime::ai::cars
