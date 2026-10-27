#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/input/directinput/directinput.hpp"
#include <cstring>

namespace nfs::services {


void create_feedback_effects(void *object) {
    if (field<int>(object, 4) < 0)
        return;
    if (field<int>(object, 8) < 0) {
        auto *effect = reinterpret_cast<Dword *>(static_cast<Byte *>(object) + 12);
        std::memset(effect, 0, 92);
        field<Dword>(object, 64) = field<Dword>(object, 76) = 4294967295u;
        *effect = 4;
        field<Dword>(object, 16) = 10000;
        field<Dword>(object, 20) = field<Dword>(object, 24) = 0;
        field<Dword>(object, 28) = 20000;
        field<Dword>(object, 68) = 100;
        field<Dword>(object, 72) = 3;
        field<Dword>(object, 80) = field<Dword>(object, 84) = 0;
        const auto *about = static_cast<Byte *>(input::device_get(field<int>(object, 4), 2));
        if (about[1] == 6)
            field<Dword>(object, 84) = 180;
        field<Dword>(object, 8) = pending::create_feedback_effect(field<int>(object, 4), effect);
    }
    const auto *about = static_cast<Byte *>(input::device_get(field<int>(object, 4), 2));
    if (about[1] == 4)
        return;
    if (field<int>(object, 200) < 0) {
        auto *effect = reinterpret_cast<Dword *>(static_cast<Byte *>(object) + 204);
        std::memset(effect, 0, 92);
        field<Dword>(object, 256) = field<Dword>(object, 268) = 4294967295u;
        *effect = 0;
        field<Dword>(object, 208) = 10000;
        field<Dword>(object, 260) = 100;
        field<Dword>(object, 264) = 3;
        field<Dword>(object, 272) = field<Dword>(object, 276) = 0;
        field<Dword>(object, 200) = pending::create_feedback_effect(field<int>(object, 4), effect);
    }
    if (field<int>(object, 104) < 0) {
        auto *effect = reinterpret_cast<Dword *>(static_cast<Byte *>(object) + 108);
        std::memset(effect, 0, 92);
        *effect = 1;
        field<Dword>(object, 112) = 10000;
        field<Dword>(object, 116) = field<Dword>(object, 160) = 0;
        field<Dword>(object, 164) = 100;
        field<Dword>(object, 168) = 3;
        field<Dword>(object, 176) = field<Dword>(object, 180) = 0;
        field<Dword>(object, 172) = 4294967295u;
        field<Dword>(object, 104) = pending::create_feedback_effect(field<int>(object, 4), effect);
    }
}

void load_force_feedback(void *object) {
    if (field<Byte>(object, 296))
        select_feedback_device(object);
}

void select_feedback_device(void *object) {
    auto release_current = [&]() {
        if (field<int>(object, 4) >= 0) {
            for (unsigned offset : {104u, 200u, 8u})
                if (field<int>(object, offset) >= 0)
                    input::stop_effect(field<int>(object, offset));
            // Stop callbacks may replace the selected device.
            input::disable_feedback(field<int>(object, 4));
            for (unsigned offset : {4u, 8u, 104u, 200u})
                field<int>(object, offset) = -1;
        }
    };
    const int count = static_cast<int>(reinterpret_cast<Dword>(input::device_get(0, 1)));
    for (int device = 0; device < count; ++device) {
        const auto *about = static_cast<input::DeviceAbout *>(input::device_get(device, 2));
        if (!about || (about->flags & 3) != 3)
            continue;
        const auto *state = static_cast<Byte *>(input::device_get(device, 6));
        if (!state || *state != 4 || !feedback_selected(device))
            continue;
        if (field<int>(object, 4) == device)
            return;
        release_current();
        if (pending::enable_feedback(device)) {
            field<int>(object, 4) = device;
            pending::set_feedback_gain(device, 100);
            create_feedback_effects(object);
            return;
        }
    }
    release_current();
}
} // namespace nfs::services
