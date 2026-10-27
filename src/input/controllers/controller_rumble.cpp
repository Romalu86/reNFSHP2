#include "nfs/input/controllers/controller_resources.hpp"
#include "nfs/input/directinput/directinput.hpp"
#include "nfs/core/services/service_dependencies.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
namespace nfs::controllers {
Dword rumble_definition[23]{};
Dword previous_rumble_definition[23]{};
Byte rumble_ready{};
int rumble_device = -1, primary_rumble_effect = -1, secondary_rumble_effect = -1;
 
namespace pending {
// The first stack word is unused by this implementation.
void set_rumble(Dword, std::uint16_t primary_strength, std::uint16_t secondary_strength) {
    if (game::runtime::at<Byte>(game::runtime::system(144), 296))
        return;
    const int device = int(get_bindings()[9]);
    if (device == -1)
        return;
    if (device != rumble_device)
        rumble_ready = 0;
    int primary_level = 0, secondary_level = 0;
    if (primary_strength) {
        const long double level =
            static_cast<long double>(int(primary_strength)) * 0.00001525902189314365386962890625f * 100.0f;
        primary_level = std::int32_t(Dword(game::runtime::hardware::truncate_integer_native80(&level)));
    }
    if (secondary_strength) {
        const long double level =
            static_cast<long double>(int(secondary_strength)) * 0.00001525902189314365386962890625f * 100.0f;
        secondary_level = std::int32_t(Dword(game::runtime::hardware::truncate_integer_native80(&level)));
    }
    if (!rumble_ready) {
        if (!services::dependency::enable_feedback(device))
            return;
        rumble_ready = 1;
        rumble_device = device;
        kernel::memory_zero(rumble_definition, 92);
        rumble_definition[0] = 4;
        rumble_definition[1] = 10000;
        rumble_definition[4] = 20000;
        rumble_definition[13] = Dword(-1);
        rumble_definition[14] = 100;
        rumble_definition[15] = 3;
        rumble_definition[17] = 0;
        rumble_definition[16] = Dword(-1);
        const auto *about = static_cast<const input::DeviceAbout *>(input::device_get(device, 2));
        const bool gamepad = reinterpret_cast<const Byte *>(about)[1] == 6;
        rumble_definition[18] = gamepad ? 45 : 0;
        const int created_effect =
            int(services::dependency::create_feedback_effect(device, rumble_definition));
        kernel::memory_copy(previous_rumble_definition, rumble_definition, 92);
        static_cast<volatile Dword *>(rumble_definition)[18] = gamepad ? 135u : 0u;
        static_cast<volatile int &>(primary_rumble_effect) = created_effect;
    }
    if (primary_rumble_effect != -1) {
        if (primary_level < 1)
            input::stop_effect(primary_rumble_effect);
        else
            input::update_effect(primary_rumble_effect, 1, nullptr, nullptr, &primary_level,
                                 nullptr, nullptr, nullptr);
    }
    if (secondary_rumble_effect != -1) {
        if (secondary_level > 0)
            input::update_effect(primary_rumble_effect, 1, nullptr, nullptr, &secondary_level,
                                 nullptr, nullptr, nullptr);
        else
            input::stop_effect(secondary_rumble_effect);
    }
}
} // namespace pending
} // namespace nfs::controllers
